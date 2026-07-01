// Minimal end-to-end example: run the authorization-code flow against
// Dropbox. Fill in your app's client key/secret and register
// http://localhost:8889/ as a redirect URI in the Dropbox App Console.
//
// Swap the endpoints and traits preset to target another provider.

#include <iostream>

#include "oauth2loopback/authorization_flow.h"

int main()
{
	oauth2loopback::AuthorizationFlow flow(
		U("YOUR_CLIENT_KEY"),
		U("YOUR_CLIENT_SECRET"),
		U("https://www.dropbox.com/oauth2/authorize"),
		U("https://api.dropboxapi.com/oauth2/token"),
		U("http://localhost:8889/"),
		oauth2loopback::providers::dropbox());

	std::cout << "Opening browser for consent..." << std::endl;
	oauth2loopback::AuthResult result = flow.authorize().get();
	flow.stop();

	if (!result.ok)
	{
		std::cerr << "Authorization failed: " << result.error << std::endl;
		return 1;
	}

	// The token is now ready to use with an http_client.
	web::http::client::http_client_config http_config;
	http_config.set_oauth2(flow.config());
	http_config.set_validate_certificates(true);

	std::cout << "Linked. Refresh token present: "
		<< (flow.config().token().refresh_token().empty() ? "no" : "yes")
		<< std::endl;
	return 0;
}
