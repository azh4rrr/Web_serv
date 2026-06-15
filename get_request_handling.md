# GET Request Handling — Full Flow with Config & Request Examples

This document walks through every branch your GET handler can take, in the
order the checks actually run in `Response::buildResponse()` /
`handleTarget()`. Each branch shows: the condition that triggers it, the
relevant config directive(s), an example request, and the response your
server should produce.

All requests below are written as `HTTP/1.0` since that's what was asked for.
One quirk to be aware of: your `Response::setStatusLine()` always writes
`HTTP/1.1` in the response status line regardless of the request's declared
version — this is technically a protocol mismatch but won't break most
clients/test tools (siege, curl, browsers all tolerate it).

---

## ASCII flow diagram

```
                    ┌──────────────────────┐
                    │   Request parsed       │
                    └───────────┬────────────┘
                                 │
                                 v
                    ┌──────────────────────┐
               Yes  │   Parse error?         │
        ┌───────────┤   (errorCode() != 0)   │
        │           └───────────┬────────────┘
        v                        │ No
┌────────────────┐               v
│ Error response   │   ┌──────────────────────┐
│ 400/501/414/505  │   │   Match location       │
└────────────────┘   │   (longest prefix)     │
                       └───────────┬────────────┘
                                    │
                                    v
                       ┌──────────────────────┐
                  No   │   GET in allow_methods?│
        ┌───────────────┤                        │
        │               └───────────┬────────────┘
        v                            │ Yes
┌────────────────┐                   v
│ 405 Method       │     ┌──────────────────────┐
│ Not Allowed      │     │   "return" directive   │
└────────────────┘      │   set on location?     │
            Yes ┌─────────┤                        │
                │         └───────────┬────────────┘
                v                      │ No
      ┌────────────────┐               v
      │ 301 Redirect     │  ┌──────────────────────┐
      │ Location: <ret>  │  │  cgi-bin / cgi_ext      │
      └────────────────┘  │  match?                │
                  Yes ┌─────┤                        │
                      │     └───────────┬────────────┘
                      v                  │ No
            ┌────────────────┐           v
            │ Run CGI script   │  ┌──────────────────────┐
            │ (handleCgi)      │  │  Resolve target path    │
            └────────────────┘  │  root/alias + URI       │
                                  └───────────┬────────────┘
                                               │
                                               v
                                  ┌──────────────────────┐
                            No    │  Is target a directory? │
                  ┌─────────────────┤                        │
                  │               └───────────┬────────────┘
                  v                            │ Yes
        ┌────────────────┐                     v
        │ Check file        │       ┌──────────────────────┐
        │ exists + readable │  No   │  URI ends with '/'?    │
        └────────┬─────────┘  ┌──────┤                        │
                  │            │     └───────────┬────────────┘
                  │            v                  │ Yes
                  │   ┌────────────────┐          v
                  │   │ 301 add slash    │  ┌──────────────────────┐
                  │   └────────────────┘  │  Append index file       │
                  │                        │  target += index        │
                  │                        └───────────┬────────────┘
                  │                                     │
                  │                                     v
                  │                        ┌──────────────────────┐
                  │                  No     │  Index file readable?  │
                  │            ┌─────────────┤                        │
                  │            v             └───────────┬────────────┘
                  │  ┌────────────────┐                   │ Yes
                  │  │  autoindex on?   │                   │
                  │  └────────┬─────────┘                   │
                  │     ┌──────┴──────┐                      │
                  │   Yes│            │No                     │
                  │      v            v                       │
                  │ ┌─────────┐ ┌──────────┐                   │
                  │ │200 listing│ │403       │                  │
                  │ └─────────┘ └──────────┘                   │
                  │                                            │
                  └──────────────────┬─────────────────────────┘
                                       v
                            ┌──────────────────────┐
                            │  Serve file             │
                            │  200 / 404 / (403)       │
                            └──────────────────────┘
```

---

## Reference config

All examples below assume this config (adapted from your `configs/default.conf`):

```nginx
server {
    listen 8002;
    server_name localhost;
    host 127.0.0.1;
    root docs/fusion_web/;
    index index.html;
    error_page 404 error_pages/404.html;
    client_max_body_size 3000000;

    location / {
        allow_methods DELETE POST GET;
        autoindex off;
    }

    location /tours {
        autoindex on;
        index tours1.html;
        allow_methods GET POST PUT HEAD;
    }

    location /red {
        return /tours;
    }

    location /cgi-bin {
        root ./;
        allow_methods GET POST DELETE;
        index time.py;
        cgi_path /usr/bin/python3 /bin/bash;
        cgi_ext .py .sh;
    }
}
```

Assume the filesystem looks like this:

```
docs/fusion_web/
├── index.html
├── about.html
├── secret.html        (chmod 000 — unreadable)
├── tours/
│   ├── tours1.html
│   └── pages/
│       └── page.html
└── error_pages/
    └── 404.html

cgi-bin/
├── time.py
└── env.py
```

---

## Case 1 — Parse error (malformed request)

**Trigger:** `request.errorCode()` is non-zero before `buildResponse()` even
calls `handleTarget()`. The request never reaches location matching.

**What sets this:** anything the `HttpRequest::feed()` state machine rejects
— bad method spelling, illegal URI character, `..` that walks above root,
URI too long, malformed version line.

**Config involved:** none — this happens before any location is consulted.

**Example request (bad character in URI):**
```
GET /tours/<script> HTTP/1.0
Host: localhost:8002
```

**Response:**
```
HTTP/1.1 400 Bad Request
Content-Type: text/html
Content-Length: 121
Date: ...
Server: AMAnix

<html>
<head><title>400 Bad Request </title></head>
<body>
<center><h1>400 Bad Request</h1></center>
```

**Example request (URI tries to escape root):**
```
GET /../../etc/passwd HTTP/1.0
Host: localhost:8002
```
`checkUriPos()` detects the `..` pushes the path count negative → `_error_code = 400`. Same response shape as above.

---

## Case 2 — Method not allowed (405)

**Trigger:** location matched successfully, but `GET` is not in that
location's `allow_methods` vector.

**Config involved:**
```nginx
location /upload {
    allow_methods POST DELETE;   # GET intentionally not listed
}
```
*(Not in the reference config above — added here to illustrate the case.
In the reference config, `/` allows GET, so to trigger 405 you'd need a
location like this.)*

**Example request:**
```
GET /upload/file.txt HTTP/1.0
Host: localhost:8002
```

**Response:**
```
HTTP/1.1 405 Method Not Allowed
Content-Type: text/html
Content-Length: 129
Date: ...
Server: AMAnix

<html>
<head><title>405 Method Not Allowed </title></head>
<body>
<center><h1>405 Method Not Allowed</h1></center>
```

Note: this check happens **before** any filesystem access — the server
doesn't reveal whether `/upload/file.txt` exists.

---

## Case 3 — `return` directive (redirect)

**Trigger:** the matched location has a non-empty `return` value.
`checkReturn()` sets `_code = 301` and `_location = return value`
(prefixing `/` if missing).

**Config involved:**
```nginx
location /red {
    return /tours;
}
```

**Example request:**
```
GET /red HTTP/1.0
Host: localhost:8002
```

**Response:**
```
HTTP/1.1 301 Moved Permanently
Content-Type: text/html
Content-Length: 0
Location: /tours
Date: ...
Server: AMAnix
```

Note: `_response_body` stays empty here because `_code != 200` is true but
`buildErrorBody()` is only triggered by `reqError() || buildBody()` returning
truthy — `checkReturn()` returns `1` from `handleTarget()`, so `buildBody()`
returns `1`, so `buildErrorBody()` *does* run. Since 301 isn't in
`_error_pages` (it's not >= 400), `setServerDefaultErrorPages()` fires and
`getErrorPage(301)` produces a small "301 Moved Permanently" HTML body — so
in practice `Content-Length` won't actually be 0. Worth testing this with
curl `-v` to confirm what your server actually sends.

---

## Case 4 — CGI execution

**Trigger:** the matched location path contains `cgi-bin`, the requested
file has an executable extension (`.py` / `.sh`), `allow_methods` permits
GET, and the script exists + is executable.

**Config involved:**
```nginx
location /cgi-bin {
    root ./;
    allow_methods GET POST DELETE;
    index time.py;
    cgi_path /usr/bin/python3 /bin/bash;
    cgi_ext .py .sh;
}
```

**Example request (explicit script):**
```
GET /cgi-bin/time.py HTTP/1.0
Host: localhost:8002
```

**Example request (directory → index script):**
```
GET /cgi-bin/ HTTP/1.0
Host: localhost:8002
```
Here `path == "cgi-bin/"`, so `handleCgi()` appends the location's
`index` (`time.py`).

**Response (from `cgi-bin/time.py`):**
```
HTTP/1.1 200 OK
Content-type: text/html

<html>
<head>
<h1>  14:32:07 </h1>
```

Things that can go wrong here, each with its own code:
- extension isn't `.py`/`.sh` → **501 Not Implemented**
- script file doesn't exist → **404 Not Found**
- script exists but not executable/readable → **403 Forbidden**
- script exits non-zero → **502 Bad Gateway** (set in `readCgiResponse()`)

---

## Case 5 — Directory without trailing slash (301)

**Trigger:** `_target_file` resolves to a directory, but the request URI
doesn't end in `/`.

**Config involved:** the `/tours` location (root inherited from server:
`docs/fusion_web/`):
```nginx
location /tours {
    autoindex on;
    index tours1.html;
    allow_methods GET POST PUT HEAD;
}
```

**Example request:**
```
GET /tours HTTP/1.0
Host: localhost:8002
```

`_target_file = docs/fusion_web/tours` → `isDirectory()` is true, last char
isn't `/` → `_code = 301`, `_location = "/tours/"`.

**Response:**
```
HTTP/1.1 301 Moved Permanently
Content-Type: text/html
Content-Length: <len of 301 error page body>
Location: /tours/
Date: ...
Server: AMAnix

<html>
<head><title>301 Moved Permanently </title></head>
<body>
<center><h1>301 Moved Permanently</h1></center>
```

Browsers follow this automatically; curl needs `-L` or you'll just see the
301.

---

## Case 6 — Directory with index file present (200)

**Trigger:** directory request ends in `/`, and `target + index` exists and
is readable.

**Config involved:** same `/tours` block — `index tours1.html`.

**Example request:**
```
GET /tours/ HTTP/1.0
Host: localhost:8002
```

`_target_file = docs/fusion_web/tours/` → append `tours1.html` →
`docs/fusion_web/tours/tours1.html` exists → `readFile()` succeeds.

**Response:**
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 842
Date: ...
Server: AMAnix

<!DOCTYPE html>
<html>
...contents of tours1.html...
```

---

## Case 7 — Directory, no index, autoindex off (403)

**Trigger:** directory request ends in `/`, `target + index` does **not**
exist, and `autoindex off` (or unset — default is off).

**Config involved:** the `/` location:
```nginx
location / {
    allow_methods DELETE POST GET;
    autoindex off;
}
```
Server-level `index index.html;` applies here. If `docs/fusion_web/`
contains an `index.html`, this case won't trigger for `/` — to demonstrate
it, imagine a sub-request to an empty directory:

```
GET /empty-dir/ HTTP/1.0
Host: localhost:8002
```

`docs/fusion_web/empty-dir/index.html` doesn't exist, `target_location`
(matched as `/`) has `autoindex == false` → `_code = 403`.

**Response:**
```
HTTP/1.1 403 Forbidden
Content-Type: text/html
Content-Length: 117
Date: ...
Server: AMAnix

<html>
<head><title>403 Forbidden </title></head>
<body>
<center><h1>403 Forbidden</h1></center>
```

---

## Case 8 — Directory, no index, autoindex on (200, listing)

**Trigger:** directory request ends in `/`, `target + index` doesn't exist,
and `autoindex on`.

**Config involved:** `/tours` has `autoindex on;`. If `tours1.html` were
missing, or for a sub-directory of `/tours` with no index:

```
GET /tours/pages/ HTTP/1.0
Host: localhost:8002
```

`docs/fusion_web/tours/pages/index.html` (the inherited server index, since
this sub-path has no location-specific index) doesn't exist → `autoindex`
inherited as `on` → `_target_file` is trimmed back to the directory and
`buildHtmlIndex()` generates a listing.

**Response:**
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 612
Date: ...
Server: AMAnix

<html>
<head>
<title> Index of docs/fusion_web/tours/pages/</title>
</head>
<body >
<h1> Index of docs/fusion_web/tours/pages/</h1>
<table style="width:80%; font-size: 15px">
<hr>
<th style="text-align:left"> File Name </th>
<th style="text-align:left"> Last Modification  </th>
<th style="text-align:left"> File Size </th>
<tr>
<td><a href="page.html">page.html</a></td>
<td>Mon Jun  9 12:01:33 2026</td>
<td>318</td>
</tr>
</table>
<hr>
</body>
</html>
```

---

## Case 9 — File not found (404)

**Trigger:** target is not a directory, and `ConfigFile::getTypePath()`
returns something other than "file" (i.e. nothing exists at that path) —
or, in the static-serving path, `readFile()`'s `ifstream` fails to open.

**Config involved:** `/` location, server root `docs/fusion_web/`.

**Example request:**
```
GET /does-not-exist.html HTTP/1.0
Host: localhost:8002
```

**Response — note the custom error page kicks in here**, because
`error_page 404 error_pages/404.html;` is configured and `docs/fusion_web/error_pages/404.html`
exists and is readable:

```
HTTP/1.1 302 Found
Content-Type: text/html
Content-Length: <len of error_pages/404.html>
Location: /error_pages/404.html
Date: ...
Server: AMAnix

...contents of error_pages/404.html...
```

This is the slightly unusual behavior in `buildErrorBody()`: for 4xx codes
with a configured error page, it sets `_location` to the error page path,
**rewrites `_code` to 302**, and serves the error page's content with that
302 status. So a missing-file request that has a custom 404 page actually
comes back as a `302 Found` carrying the 404 page's HTML — not as a `404`.
If your grader/tests expect a literal `404` status line with the custom
page's body, this is a mismatch worth fixing (e.g., keep `_code = 404` and
just swap `_response_body`, without touching `_location`/302).

**If no `error_page 404` were configured** (or the file were missing), you'd
get the plain default:
```
HTTP/1.1 404 Not Found
Content-Type: text/html
Content-Length: 110
Date: ...
Server: AMAnix

<html>
<head><title>404 Not Found </title></head>
<body>
<center><h1>404 Not Found</h1></center>
```

---

## Case 10 — File exists but unreadable (403, in theory)

**Trigger (intended):** target resolves to a real file, but the process
lacks read permission.

**Config involved:** `/` location.

**Example request:**
```
GET /secret.html HTTP/1.0
Host: localhost:8002
```
where `docs/fusion_web/secret.html` exists with `chmod 000`.

**Current behavior in your code:** `handleTarget()` doesn't call
`access()`/`checkFile()` for plain static GETs (it does for CGI scripts, but
not for normal files). It goes straight to `readFile()`, which opens an
`ifstream`. On most systems an `ifstream::open()` on a permission-denied
file fails the same way as "file doesn't exist" — so `readFile()` returns 1
and `_code` gets set to **404**, not 403.

**Actual response you'd currently get:**
```
HTTP/1.1 404 Not Found
...
```
(or the 302-wrapped custom 404 page, per Case 9, since the same
`error_page 404` applies)

**To get a proper 403** you'd add, before `readFile()` in the GET/HEAD
branch of `buildBody()`:
```cpp
if (ConfigFile::checkFile(_target_file, 4) == -1)  // R_OK = 4
{
    _code = 403;
    return (1);
}
```
which mirrors the check `handleCgi()` already does for scripts.

---

## Case 11 — Normal static file, GET vs HEAD (200)

**Trigger:** target is a regular, existing, readable file.

**Config involved:** `/` location, server root `docs/fusion_web/`.

**Example GET request:**
```
GET /about.html HTTP/1.0
Host: localhost:8002
```

**Response (GET — body included):**
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1530
Date: ...
Server: AMAnix

<!DOCTYPE html>
<html>
...full contents of about.html...
```

**Example HEAD request** (`/tours` location allows HEAD):
```
HEAD /tours/tours1.html HTTP/1.0
Host: localhost:8002
```

**Response (HEAD — same headers, no body):**
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 842
Date: ...
Server: AMAnix

```
`Content-Length` still reflects the full file size (computed from
`_response_body.length()` before the body is withheld) — this is correct
per spec, since `Content-Length` for HEAD should match what GET would return.
The body itself is omitted because `buildResponse()` checks
`request.getMethod() != HEAD` before appending `_response_body`.

---

## Quick-reference table

| Case | Status | Config directive that matters | Filesystem state |
|---|---|---|---|
| 1 | 400/501/414/505 | none | n/a (parse-time) |
| 2 | 405 | `allow_methods` missing GET | n/a |
| 3 | 301 | `return` | n/a |
| 4 | 200 (CGI) | `cgi_path`, `cgi_ext`, `index` | script exists, executable |
| 5 | 301 | (any) | target is dir, URI has no `/` |
| 6 | 200 | `index` | dir + index file exists |
| 7 | 403 | `autoindex off` | dir, no index file |
| 8 | 200 (listing) | `autoindex on` | dir, no index file |
| 9 | 404 (or 302-wrapped) | `error_page 404 <path>` | path doesn't exist |
| 10 | 404 (should be 403) | n/a | file exists, unreadable |
| 11 | 200 | n/a | normal file, GET/HEAD |

---

## Suggested test sequence with `curl`

```bash
# Case 2 — method not allowed (needs a location restricting GET)
curl -i http://127.0.0.1:8002/upload/x

# Case 3 — redirect
curl -i http://127.0.0.1:8002/red

# Case 4 — CGI
curl -i http://127.0.0.1:8002/cgi-bin/time.py

# Case 5 — directory, no trailing slash
curl -i http://127.0.0.1:8002/tours

# Case 6 — directory with index
curl -i http://127.0.0.1:8002/tours/

# Case 8 — autoindex listing
curl -i http://127.0.0.1:8002/tours/pages/

# Case 9 — 404 with custom error page
curl -i http://127.0.0.1:8002/nope.html

# Case 11 — normal file
curl -i http://127.0.0.1:8002/about.html

# Case 11 — HEAD
curl -i -X HEAD http://127.0.0.1:8002/tours/tours1.html
```

`curl -i` shows status line + headers, which is what you need to verify
against the table above. Add `-v` if you want to confirm the request line
your server actually receives.
