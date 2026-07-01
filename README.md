# oauth2loopback — Native App OAuth 2.0 via Loopback Listener (C++)

A small C++ library for completing the **OAuth 2.0 Authorization Code flow in a desktop application** — no embedded webview, no copy-pasting codes. The app opens the user's real browser for consent, then captures the redirect on a `http://localhost` loopback listener and exchanges it for an access token.

This is the approach recommended by [RFC 8252 — OAuth 2.0 for Native Apps](https://datatracker.ietf.org/doc/html/rfc8252): use the system browser and a loopback redirect URI rather than an embedded user-agent.

## Usage

```cpp
#include "oauth2loopback/authorization_flow.h"

oauth2loopback::AuthorizationFlow flow(
    U("CLIENT_KEY"), U("CLIENT_SECRET"),
    U("https://www.dropbox.com/oauth2/authorize"),
    U("https://api.dropboxapi.com/oauth2/token"),
    U("http://localhost:8889/"),               // registered loopback redirect URI
    oauth2loopback::providers::dropbox());      // provider quirks preset

oauth2loopback::AuthResult result = flow.authorize().get(); // or .then(...)
flow.stop();

if (result.ok) {
    web::http::client::http_client_config http_config;
    http_config.set_oauth2(flow.config());      // token is in flow.config().token()
}
```

`authorize()` builds the authorization URL, opens the system browser, and returns a `pplx::task<AuthResult>` that resolves once the loopback listener has captured the redirect and finished the token exchange. On failure, `AuthResult::error` says why.

A runnable version of this lives in [`examples/dropbox_example.cpp`](examples/dropbox_example.cpp).

## Provider quirks as data

Providers deviate from the vanilla flow in small ways. Instead of hardcoding per-provider branches, quirks are described by a `ProviderTraits` value:

| Field | Meaning |
|-------|---------|
| `extra_auth_params` | Extra query parameters on the authorization URL (e.g. `access_type=offline` for a Google refresh token) |
| `generate_state` | Generate/verify the `state` parameter — disable for providers that don't echo it back |
| `extract_code_manually` | Pull `code` out of the redirect and use `token_from_code()` instead of `token_from_redirected_uri()` |
| `success_body` / `failure_body` | Page shown in the browser after the redirect |

Presets are included for the providers this was originally battle-tested against:

- `providers::dropbox()` — standard flow, requests a refresh token via `token_access_type=offline`.
- `providers::google_drive()` — requests `access_type=offline`; extracts the `code` manually because the raw redirect can't be fed straight into `token_from_redirected_uri`.
- `providers::pcloud()` — does not echo `state` back, so state verification is disabled and the `code` is extracted manually.

Any other provider is a `ProviderTraits` value away — no library changes needed.

## Layout

| Path | Responsibility |
|------|----------------|
| `include/oauth2loopback/authorization_flow.h` | `AuthorizationFlow` — builds the auth URL, opens the browser, awaits the result. Also `open_in_default_browser()`. |
| `include/oauth2loopback/code_listener.h` | `CodeListener` — loopback `http_listener` that captures the redirect and drives the token exchange. |
| `include/oauth2loopback/provider_traits.h` | `ProviderTraits` and the provider presets. |
| `examples/` | Runnable end-to-end example. |

## Building

Depends on the [C++ REST SDK (cpprestsdk)](https://github.com/microsoft/cpprestsdk), available via vcpkg (a `vcpkg.json` manifest is included):

```sh
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

Or consume the library directly from your own CMake project:

```cmake
add_subdirectory(native-oauth2-loopback-cpp)
target_link_libraries(your_app PRIVATE oauth2loopback)
```

By default the browser is opened with `ShellExecute` (Windows), `open` (macOS), or `xdg-open` (Linux); override it with `set_browser_launcher()` if your app needs to show a prompt first or launch differently.

## Limitations

- **No PKCE.** RFC 8252 recommends PKCE for native apps, but cpprestsdk's experimental `oauth2_config` does not support it. If your provider requires PKCE you'll need to extend the token exchange.
- The redirect port is fixed by the `redirect_uri` you register; there's no automatic free-port selection yet.
- cpprestsdk's OAuth 2.0 support lives in an `experimental` namespace upstream.

## License

MIT — see [LICENSE](LICENSE). Original code © Penjani Mkandawire.
