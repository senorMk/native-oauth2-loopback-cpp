#pragma once

#include <map>

#include "cpprest/details/basic_types.h"

namespace oauth2loopback {

// Describes the ways a provider deviates from the vanilla OAuth 2.0
// authorization-code flow. Fill one in for your provider, or start from one
// of the presets in oauth2loopback::providers.
struct ProviderTraits
{
	// Extra query parameters appended to the authorization URL, e.g.
	// {"access_type", "offline"} to request a refresh token from Google.
	std::map<utility::string_t, utility::string_t> extra_auth_params;

	// Generate and verify a `state` parameter on the redirect. Disable for
	// providers that do not echo `state` back (e.g. pCloud), where
	// verification would always fail.
	bool generate_state = true;

	// Extract the `code` parameter from the redirect manually and exchange it
	// with token_from_code() instead of handing the whole redirect URI to
	// token_from_redirected_uri(). Needed when the redirect cannot be
	// consumed verbatim (Google Drive) or omits `state` (pCloud).
	bool extract_code_manually = false;

	// Page body shown in the user's browser once the redirect is handled.
	utility::string_t success_body = U("Authorization complete. You can close this window.");
	utility::string_t failure_body = U("Authorization failed. You can close this window.");
};

// Presets for providers whose quirks this library was originally written
// around. Any other provider is just a ProviderTraits value away.
namespace providers {

inline ProviderTraits dropbox()
{
	ProviderTraits traits;
	// Ask Dropbox to issue a refresh token alongside the access token.
	traits.extra_auth_params[U("token_access_type")] = U("offline");
	return traits;
}

inline ProviderTraits google_drive()
{
	ProviderTraits traits;
	traits.extra_auth_params[U("access_type")] = U("offline");
	// The raw redirect cannot be fed straight into token_from_redirected_uri.
	traits.extract_code_manually = true;
	return traits;
}

inline ProviderTraits pcloud()
{
	ProviderTraits traits;
	// pCloud does not echo `state` back on the redirect.
	traits.generate_state = false;
	traits.extract_code_manually = true;
	return traits;
}

} // namespace providers
} // namespace oauth2loopback
