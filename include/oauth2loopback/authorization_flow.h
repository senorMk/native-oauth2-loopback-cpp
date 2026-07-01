#pragma once

#include <functional>
#include <memory>

#include "oauth2loopback/code_listener.h"

namespace oauth2loopback {

// Opens `url` in the platform's default browser
// (ShellExecute / open / xdg-open).
void open_in_default_browser(const utility::string_t& url);

// Runs the RFC 8252 authorization-code flow for a native app: builds the
// authorization URL, opens the system browser for consent, and resolves once
// the loopback listener has captured the redirect and exchanged the code for
// a token.
//
//   oauth2loopback::AuthorizationFlow flow(key, secret,
//       auth_endpoint, token_endpoint,
//       U("http://localhost:8889/"),
//       oauth2loopback::providers::dropbox());
//   flow.authorize().then([&](oauth2loopback::AuthResult r) { ... });
//
class AuthorizationFlow
{
	public:
		AuthorizationFlow(utility::string_t client_key,
			utility::string_t client_secret,
			utility::string_t auth_endpoint,
			utility::string_t token_endpoint,
			utility::string_t redirect_uri,
			ProviderTraits traits = {});

		// Override how the authorization URL is opened; defaults to the
		// system browser. Useful for tests or apps that show a prompt first.
		void set_browser_launcher(std::function<void(const utility::string_t&)> launcher);

		// Starts the flow. The returned task completes when the token
		// exchange finishes; on success config().token() holds the token.
		// If a valid access token is already present, completes immediately.
		pplx::task<AuthResult> authorize();

		// Shuts down the loopback listener. Call once authorize() resolves
		// (or to abort a pending flow).
		void stop() { m_listener->stop(); }

		web::http::oauth2::experimental::oauth2_config& config() { return m_config; }
		const web::http::oauth2::experimental::oauth2_config& config() const { return m_config; }

	private:
		utility::string_t build_auth_uri();

		web::http::oauth2::experimental::oauth2_config m_config;
		ProviderTraits m_traits;
		std::unique_ptr<CodeListener> m_listener;
		std::function<void(const utility::string_t&)> m_launcher;
};

} // namespace oauth2loopback
