#pragma once

#include <memory>
namespace web {
	namespace http {
		namespace experimental {
			namespace listener {
				class http_listener;
			}
		}
	}
}

enum class CloudService { Dropbox, pCloud, GoogleDrive, Box, OneDrive};

#include "cpprest/http_client.h"
#include "cpprest/http_listener.h"

//
// A simple listener class to capture OAuth 2.0 HTTP redirect to localhost.
// The listener captures redirected URI and obtains the token.
// This type of listener can be implemented in the back-end to capture and store tokens.
//
class OAuth2CodeListener
{
	public:

		OAuth2CodeListener(web::uri listen_uri, web::http::oauth2::experimental::oauth2_config& config, CloudService service);
		~OAuth2CodeListener();

		void StopListener();
		// Create the listening task
		pplx::task<bool> listen_for_code();

	private:
		std::unique_ptr<web::http::experimental::listener::http_listener> m_listener;
		pplx::task_completion_event<bool> m_tce;
		web::http::oauth2::experimental::oauth2_config& m_config;
		std::mutex m_resplock;
		CloudService m_service;
};