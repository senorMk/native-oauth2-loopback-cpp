#include "OAuth2CodeListener.h"


OAuth2CodeListener::OAuth2CodeListener(web::uri listen_uri, web::http::oauth2::experimental::oauth2_config& config, CloudService service)
	: m_listener(new web::http::experimental::listener::http_listener(listen_uri)), m_config(config), m_service(service)
{
	m_listener->support([this](web::http::http_request request) -> void
	{
		if (request.request_uri().path() == U("/") && request.request_uri().query() != U(""))
		{
			// Lock the mutex, get exclusive access
			m_resplock.lock();

			if (m_service == CloudService::Dropbox)
			{
				// Listen for the authorization code sent to our listener
				bool implicit_grant = m_config.implicit_grant();
				m_config.token_from_redirected_uri(request.request_uri()).then([=](pplx::task<void> token_task)
				{
					// Stop waiting, set task completion to true
					token_task.wait();
					m_tce.set(true);
				}
				).then([this, request](pplx::task<void> token_task)
				{
					// Catch any lethal exceptions
					if (token_task._GetImpl()->_HasUserException()) {
						auto holder = token_task._GetImpl()->_GetExceptionHolder(); // Probably should put in try

						try {
							try
							{
								// Need to make sure you try/catch here, as _RethrowUserException can throw
								holder->_RethrowUserException();
							}
							catch (std::exception& e)
							{
								// TODO: log the caught exception (e.what()) with your own logger.
							}
						}
						catch (std::exception& e) {
							// Do what you need to do here
								// TODO: log the caught exception (e.what()) with your own logger.
							m_tce.set(false);
						}
					}
				});
			}
			// Highlight: workaround for Google Drive - malformed auth code
			else if (m_service == CloudService::GoogleDrive)
			{
				auto query = web::http::uri::split_query((m_config.implicit_grant()) ? request.request_uri().fragment() : request.request_uri().query());
				auto code_param = query.find(U("code"));

				if (code_param != query.end())
				{
					utility::string_t auth_code = web::uri::decode(code_param->second);

					// Listen for the authorization code sent to our listener
					m_config.token_from_code(auth_code).then([=](pplx::task<void> token_task)
					{
						// Stop waiting, set task completion to true
						token_task.wait();
						m_tce.set(true);
					}
					).then([this, request](pplx::task<void> token_task)
					{
						// Catch any lethal exceptions
						if (token_task._GetImpl()->_HasUserException()) {
							auto holder = token_task._GetImpl()->_GetExceptionHolder(); // Probably should put in try

							try {
								try
								{
									// Need to make sure you try/catch here, as _RethrowUserException can throw
									holder->_RethrowUserException();
								}
								catch (std::exception& e) {
								// TODO: log the caught exception (e.what()) with your own logger.
								}
							}
							catch (std::exception& e) {
								// Do what you need to do here
								// TODO: log the caught exception (e.what()) with your own logger.
								m_tce.set(false);
							}
						}
					});
				}
			}
			// Highlight: workaround for pCloud not returning a state parameter with it's query
			else if (m_service == CloudService::pCloud)
			{
				auto query = web::http::uri::split_query((m_config.implicit_grant()) ? request.request_uri().fragment() : request.request_uri().query());
				auto code_param = query.find(U("code"));
				if (code_param != query.end())
				{
					// Listen for the authorization code sent to our listener
					m_config.token_from_code(code_param->second).then([=](pplx::task<void> token_task)
					{
						// Stop waiting, set task completion to true
						token_task.wait();
						m_tce.set(true);
					}
					).then([this, request](pplx::task<void> token_task)
					{
						// Catch any lethal exceptions
						if (token_task._GetImpl()->_HasUserException()) {
							auto holder = token_task._GetImpl()->_GetExceptionHolder(); // Probably should put in try

							try {
								try
								{
									// Need to make sure you try/catch here, as _RethrowUserException can throw
									holder->_RethrowUserException();
								}
								catch (std::exception& e) {
								// TODO: log the caught exception (e.what()) with your own logger.
								}
							}
							catch (std::exception& e) {
								// Do what you need to do here
								// TODO: log the caught exception (e.what()) with your own logger.
								m_tce.set(false);
							}
						}
					});
				}
			}

			switch (m_service)
			{
				case CloudService::Dropbox:
				{
					request.reply(web::http::status_codes::OK);
					//request.reply(web::http::status_codes::OK, U("This app is now linked to your Dropbox account."));
				}
				break;
				case CloudService::pCloud:
				{
					request.reply(web::http::status_codes::OK);
				}
				break;
				case CloudService::GoogleDrive:
				{
					request.reply(web::http::status_codes::OK);
				}
				break;
				case CloudService::Box:
				{
					request.reply(web::http::status_codes::OK);
				}
				break;
				case CloudService::OneDrive:
				{
					request.reply(web::http::status_codes::OK);
				}
				break;
				default:
					wxASSERT(false);
					break;
			}

			m_resplock.unlock();
		}
		// if not (request.request_uri().path() == U("/") && request.request_uri().query() != U(""))
		else
		{

		}
	});

	m_listener->open().wait();
}

OAuth2CodeListener::~OAuth2CodeListener()
{
	m_listener->close().wait();
}

void OAuth2CodeListener::StopListener()
{
	m_listener->close().wait();
}

pplx::task<bool> OAuth2CodeListener::listen_for_code()
{
	return pplx::create_task(m_tce);
}