#pragma once

#include "OAuth2CodeListener.h"

enum class ConnectedService { Dropbox, pCloud, GoogleDrive };

// Base class for OAuth 2.0 session
//
class OAuth2Session
{
	public:
		OAuth2Session(utility::string_t client_key, utility::string_t client_secret,
			utility::string_t auth_endpoint, utility::string_t token_endpoint, utility::string_t redirect_uri,
			CloudService service);

	protected:
		void DoLinkProcess(ConnectedService serv, bool GenerateState);
		virtual void BeginLinkProcess() = 0;
		virtual void CreateBackupsBaseFolder() = 0;
		pplx::task<bool> authorization_code_flow(ConnectedService serv, bool GenerateState);
		void StopListener() { m_listener->StopListener(); }

	protected:
		web::http::client::http_client_config m_http_config;
		web::http::oauth2::experimental::oauth2_config m_oauth2_config;

	private:
		bool is_enabled() const;
		void open_browser_auth(ConnectedService serv, bool GenerateState);

	private:
		std::unique_ptr<OAuth2CodeListener> m_listener;
};

