#pragma once

#include <memory>
#include <mutex>
#include <string>

#include "cpprest/http_client.h"
#include "cpprest/http_listener.h"

#include "oauth2loopback/provider_traits.h"

namespace oauth2loopback {

// Outcome of the authorization flow. `error` is empty on success.
struct AuthResult
{
	bool ok = false;
	std::string error;
};

// Loopback http_listener that captures the OAuth 2.0 redirect on
// http://localhost:PORT/ and exchanges the authorization code for a token.
// The token lands in the oauth2_config passed to the constructor.
class CodeListener
{
	public:
		CodeListener(web::uri listen_uri,
			web::http::oauth2::experimental::oauth2_config& config,
			ProviderTraits traits);
		~CodeListener();

		// Task that completes once the redirect has been handled and the
		// token exchange has finished (successfully or not).
		pplx::task<AuthResult> result() const { return pplx::create_task(m_tce); }

		void stop();

	private:
		void handle_request(web::http::http_request request);

		std::unique_ptr<web::http::experimental::listener::http_listener> m_listener;
		web::http::oauth2::experimental::oauth2_config& m_config;
		ProviderTraits m_traits;
		pplx::task_completion_event<AuthResult> m_tce;
		std::mutex m_mutex;
};

} // namespace oauth2loopback
