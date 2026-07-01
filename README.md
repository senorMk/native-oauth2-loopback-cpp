# Native App OAuth 2.0 via Loopback Listener (C++)

A small, self-contained pattern for completing the **OAuth 2.0 Authorization Code flow in a desktop application** — no embedded webview, no copy-pasting codes. The app opens the user's real browser for consent, then captures the redirect on a `http://localhost` loopback listener and exchanges it for an access token.

This is the approach recommended by [RFC 8252 — OAuth 2.0 for Native Apps](https://datatracker.ietf.org/doc/html/rfc8252): use the system browser and a loopback redirect URI rather than an embedded user-agent.

## How it works

1. `OAuth2Session::open_browser_auth()` builds the provider authorization URL (adding `access_type=offline` / `token_access_type=offline` so a refresh token is issued) and launches the system browser.
2. The user approves access in the browser.
3. The provider redirects to the app's loopback URI (`http://localhost:PORT/`). `OAuth2CodeListener` is an `http_listener` bound to that URI; it receives the request, extracts the `code`, and exchanges it for a token.
4. Completion is signalled through a `pplx::task_completion_event<bool>`, so the caller can await/`.then()` the result cleanly.

## Files

| File | Responsibility |
|------|----------------|
| `OAuth2CodeListener.h/.cpp` | Loopback `http_listener` that captures the redirect and drives the token exchange. Signals completion via a task-completion event. |
| `OAuth2Session.h/.cpp` | Base session: builds the auth URL, opens the browser, waits for the listener, stores the token config. Subclass and implement `BeginLinkProcess()` per provider. |

## Provider quirks handled

- **Google Drive** — parses the `code` out of the query/fragment manually because the raw redirect can be awkward to feed straight into `token_from_redirected_uri`.
- **pCloud** — does not echo back the `state` parameter, so it's handled via `token_from_code` on the extracted `code`.
- **Dropbox** — uses the standard `token_from_redirected_uri` path.

## Dependencies

- [C++ REST SDK (cpprestsdk / Casablanca)](https://github.com/microsoft/cpprestsdk) — `http_listener`, `http_client`, and the `oauth2` experimental config/token helpers.
- The original application's own logging/config helpers and wxWidgets (`wxString`, `wxLaunchDefaultBrowser`). Swap in your own logging, config, and "open browser" call to reuse the pattern — the spots that need them are marked with `TODO` comments.

## Status

A **reference extraction**, published to share the pattern — not a build-out-of-the-box library. The listener and session logic are the reusable parts; the app hooks are left visible so you can see where to plug in your own.

## License

MIT — see [LICENSE](LICENSE). Original code © Penjani Mkandawire.
