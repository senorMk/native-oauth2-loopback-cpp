#include "OAuth2Session.h"

#include "cpprest/filestream.h"

OAuth2Session::OAuth2Session(utility::string_t client_key, utility::string_t client_secret, utility::string_t auth_endpoint,
	utility::string_t token_endpoint, utility::string_t redirect_uri, CloudService service)
	: m_oauth2_config(client_key, client_secret, auth_endpoint, token_endpoint, redirect_uri),
	m_listener(new OAuth2CodeListener(redirect_uri, m_oauth2_config, service))
{
	// Set default timeout to 60 seconds
	m_http_config.set_timeout(std::chrono::seconds(60));
}

void OAuth2Session::DoLinkProcess(ConnectedService serv, bool GenerateState)
{
	if (is_enabled())
	{
		if (!m_oauth2_config.token().is_valid_access_token())
		{
			// Use the OAuth 2.0 Authorization Code Flow to authentication
			authorization_code_flow(serv, GenerateState).then([=](pplx::task<bool> task)
			{
				if (task.get())
				{
					// Access Token has been acquired
					m_http_config.set_oauth2(m_oauth2_config);
					m_http_config.set_validate_certificates(true);
					m_listener->StopListener();

					BeginLinkProcess();
					CreateBackupsBaseFolder();
				}
				else
				{
					// TODO: notify the user that authorization failed (access denied).
				}
			});
		}
	}
	else
	{
		// TODO: log that the session is misconfigured (client key/secret empty).
	}
}

pplx::task<bool> OAuth2Session::authorization_code_flow(ConnectedService serv, bool GenerateState)
{
	open_browser_auth(serv, GenerateState);
	return m_listener->listen_for_code();
}

bool OAuth2Session::is_enabled() const
{
	return !m_oauth2_config.client_key().empty() && !m_oauth2_config.client_secret().empty();
}

void OAuth2Session::open_browser_auth(ConnectedService serv, bool GenerateState)
{
	auto auth_uri(m_oauth2_config.build_authorization_uri(GenerateState));
	utility::string_t final_;

	wxString Caption;
	switch (serv)
	{
		case ConnectedService::Dropbox:
		{
			Caption = wxString::Format(_("Dropbox Account Setup"));

			web::uri_builder ub(auth_uri);
			ub.append_query(L"token_access_type", "offline");
			final_ = ub.to_string();
		}
		break;
		case ConnectedService::pCloud:
		{
			Caption = wxString::Format(_("pCloud Account Setup"));
			final_ = auth_uri;
		}
		break;
		case ConnectedService::GoogleDrive:
		{
			Caption = wxString::Format(_("Google Drive Account Setup"));

			web::uri_builder ub(auth_uri);
			ub.append_query(L"access_type", "offline");
			final_ = ub.to_string();
		}
		break;
		default:
			break;
	}

	wxLaunchDefaultBrowser(final_);
}