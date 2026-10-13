#!/bin/sh
# Attach the packages in dist/ to the Gitea release for this tag.
#
#   scripts/ci-upload.sh                  every package in dist/
#   scripts/ci-upload.sh dist/one.zip     just these
#
# Authenticates with the `gitea_token` secret when there is one, and otherwise with the
# credential Woodpecker gives every step for cloning.
#
# Creates the release when the tag has none, with RELEASE_NOTES.md as its body: a
# `vX.Y.Z` tag starts the pipeline, which builds, tests and publishes the release.
set -eu

FORGE="${FORGE_API:-https://git.teletypegames.org/api/v1}"
REPO="${REPO:-${CI_REPO:-}}"
TAG="${TAG:-${CI_COMMIT_TAG:-}}"
DIST="${DIST:-dist}"
NOTES="${NOTES:-RELEASE_NOTES.md}"

say() { echo "[ci-upload] $*"; }
die() { echo "[ci-upload] error: $*" >&2; exit 1; }

# A `gitea_token` secret wins; otherwise the cloning credential (an access token of the
# repository's owner). Gitea takes a personal access token as `token …` and an OAuth one
# as `Bearer …`, so the scheme is probed rather than assumed.
TOKEN="${GITEA_TOKEN:-${CI_NETRC_PASSWORD:-}}"
[ -n "$TOKEN" ] || die "no credential: set GITEA_TOKEN, or run this where Woodpecker provides CI_NETRC_PASSWORD"
[ -n "$REPO" ] || die "cannot work out the repository — set REPO=owner/name"
[ -n "$TAG" ] || die "cannot work out the tag — set TAG=v1.2.3"

AUTH=""
for scheme in token Bearer; do
  if curl -fsS -H "Authorization: $scheme $TOKEN" "$FORGE/user" >/dev/null 2>&1; then
    AUTH="Authorization: $scheme $TOKEN"
    say "authenticated with the $scheme scheme"
    break
  fi
done
[ -n "$AUTH" ] || die "the credential was refused by $FORGE — it cannot read /user"

api() {
  method="$1"; path="$2"; shift 2
  curl -fsS -X "$method" -H "$AUTH" "$FORGE$path" "$@"
}

# One path per line, read with `while IFS= read -r`, so a name with a space still works.
LIST="$(mktemp)"
trap 'rm -f "$LIST"' EXIT
if [ "$#" -gt 0 ]; then
  for given in "$@"; do printf '%s\n' "$given"; done > "$LIST"
else
  find "$DIST" -maxdepth 1 -type f \( -name '*.zip' -o -name '*.EXE' \) 2>/dev/null | sort > "$LIST" || true
fi
[ -s "$LIST" ] || die "no packages in $DIST (run scripts/ci-package.sh first)"

say "$REPO $TAG"

# A missing release answers 404, which `curl -f` would fail on, so the lookup is judged
# by what came back.
release_id="$(curl -sS -H "$AUTH" "$FORGE/repos/$REPO/releases/tags/$TAG" | jq -r '.id // empty')"

if [ -z "$release_id" ]; then
  say "no release for $TAG yet — creating it"
  notes=''
  [ -f "$NOTES" ] && notes="$(cat "$NOTES")"
  payload="$(jq -n --arg tag "$TAG" --arg title "BMAGIC $TAG" --arg body "$notes" \
    '{tag_name: $tag, name: $title, body: $body, draft: false, prerelease: false}')"
  release_id="$(api POST "/repos/$REPO/releases" \
    -H 'Content-Type: application/json' -d "$payload" | jq -r '.id // empty')"
  [ -n "$release_id" ] || die "the release for $TAG could not be created"
else
  say "the release already exists"
fi

while IFS= read -r asset; do
  [ -n "$asset" ] || continue
  [ -f "$asset" ] || die "no such file: $asset"
  name="$(basename "$asset")"
  encoded="$(printf '%s' "$name" | jq -sRr @uri)"

  # Replace rather than refuse, so re-running a build lands.
  existing="$(api GET "/repos/$REPO/releases/$release_id/assets" |
    jq -r --arg name "$name" '.[] | select(.name == $name) | .id')"
  for id in $existing; do
    say "replacing $name"
    api DELETE "/repos/$REPO/releases/$release_id/assets/$id" >/dev/null
  done

  say "uploading $name"
  api POST "/repos/$REPO/releases/$release_id/assets?name=$encoded" \
    -F "attachment=@$asset" >/dev/null
done < "$LIST"

say "done:"
api GET "/repos/$REPO/releases/$release_id" |
  jq -r '.assets[] | "  \(.name)  \(.size) bytes"'
