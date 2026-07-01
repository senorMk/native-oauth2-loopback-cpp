#include "oauth2loopback/authorization_flow.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#else
#include <cstdlib>
#endif

namespace oauth2loopback {

void open_in_default_browser(const utility::string_t& url)
{
#ifdef _WIN32
	ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
	std::string command = "open \"" + url + "\"";
	std::system(command.c_str());
#else
	std::string command = "xdg-open \"" + url + "\"";
	std::system(command.c_str());
#endif
}

AuthorizationFlow::AuthorizationFlow(utility::string_t client_key,
	utility::string_t client_secret,
	utility::string_t auth_endpoint,
	utility::string_t token_endpoint,
	utility::string_t redirect_uri,
	ProviderTraits traits)
	: m_config(std::move(client_key), std::move(client_secret),
		std::move(auth_endpoint), std::move(token_endpoint), redirect_uri),
	m_traits(std::move(traits)),
	m_listener(new CodeListener(redirect_uri, m_config, m_traits)),
	m_launcher(open_in_default_browser)
{
}

void AuthorizationFlow::set_browser_launcher(std::function<void(const utility::string_t&)> launcher)
{
	m_launcher = std::move(launcher);
}

pplx::task<AuthResult> AuthorizationFlow::authorize()
{
	if (m_config.client_key().empty() || m_config.client_secret().empty())
	{
		return pplx::task_from_result(AuthResult{ false, "client key/secret not configured" });
	}

	if (m_config.token().is_valid_access_token())
	{
		return pplx::task_from_result(AuthResult{ true, {} });
	}

	m_launcher(build_auth_uri());
	return m_listener->result();
}

utility::string_t AuthorizationFlow::build_auth_uri()
{
	web::uri_builder builder(m_config.build_authorization_uri(m_traits.generate_state));
	for (const auto& param : m_traits.extra_auth_params)
	{
		builder.append_query(param.first, param.second);
	}
	return builder.to_string();
}

} // namespace oauth2loopback
