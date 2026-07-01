#include "oauth2loopback/code_listener.h"

namespace oauth2loopback {

CodeListener::CodeListener(web::uri listen_uri,
	web::http::oauth2::experimental::oauth2_config& config,
	ProviderTraits traits)
	: m_listener(new web::http::experimental::listener::http_listener(std::move(listen_uri))),
	m_config(config),
	m_traits(std::move(traits))
{
	m_listener->support([this](web::http::http_request request)
	{
		handle_request(std::move(request));
	});
	m_listener->open().wait();
}

CodeListener::~CodeListener()
{
	stop();
}

void CodeListener::stop()
{
	m_listener->close().wait();
}

void CodeListener::handle_request(web::http::http_request request)
{
	// Ignore anything that is not the redirect we are waiting for
	// (favicon requests, stray probes, ...).
	if (request.request_uri().path() != U("/") || request.request_uri().query().empty())
	{
		request.reply(web::http::status_codes::NotFound);
		return;
	}

	std::lock_guard<std::mutex> lock(m_mutex);

	pplx::task<void> exchange;
	if (m_traits.extract_code_manually)
	{
		auto query = web::http::uri::split_query(m_config.implicit_grant()
			? request.request_uri().fragment()
			: request.request_uri().query());
		auto code_param = query.find(U("code"));
		if (code_param == query.end())
		{
			request.reply(web::http::status_codes::BadRequest, m_traits.failure_body);
			m_tce.set(AuthResult{ false, "redirect did not contain an authorization code" });
			return;
		}
		exchange = m_config.token_from_code(web::uri::decode(code_param->second));
	}
	else
	{
		exchange = m_config.token_from_redirected_uri(request.request_uri());
	}

	exchange.then([this, request](pplx::task<void> token_task)
	{
		AuthResult result;
		try
		{
			token_task.get();
			result.ok = true;
		}
		catch (const std::exception& e)
		{
			result.error = e.what();
		}

		request.reply(web::http::status_codes::OK,
			result.ok ? m_traits.success_body : m_traits.failure_body);
		m_tce.set(std::move(result));
	});
}

} // namespace oauth2loopback
