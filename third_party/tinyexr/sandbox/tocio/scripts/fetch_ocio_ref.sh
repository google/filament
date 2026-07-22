#!/usr/bin/env bash
#
# fetch_ocio_ref.sh — download reference material for validating the tocio
# OCIO engine against the upstream OpenColorIO project.
#
# Fetches into sandbox/tocio/ref/ (gitignored, NOT a submodule):
#   ref/OpenColorIO/              shallow clone of the OCIO C++ reference library
#   ref/OpenColorIO-Config-ACES/  shallow clone of the ACES config build scripts
#   ref/configs/                  pre-built *.ocio ACES configs from the release
#
# Re-running updates existing clones in place (idempotent).
#
# Usage:
#   ./fetch_ocio_ref.sh                 # pinned stable refs (see defaults below)
#   OCIO_REF=main ACES_REF=main ./fetch_ocio_ref.sh   # bleeding-edge
#   ./fetch_ocio_ref.sh --no-configs    # skip the pre-built .ocio downloads
#
set -euo pipefail

# --- pinned defaults (override via env) -------------------------------------
# OpenColorIO library v2.5.x pairs with the ACES config v4.0.0 (built for ocio-v2.5).
OCIO_REF="${OCIO_REF:-v2.5.2}"
ACES_REF="${ACES_REF:-v4.0.0}"
# Release tag that hosts the pre-built .ocio config assets.
ACES_CONFIG_RELEASE="${ACES_CONFIG_RELEASE:-v4.0.0}"

OCIO_URL="https://github.com/AcademySoftwareFoundation/OpenColorIO.git"
ACES_URL="https://github.com/AcademySoftwareFoundation/OpenColorIO-Config-ACES.git"
RELEASE_BASE="https://github.com/AcademySoftwareFoundation/OpenColorIO-Config-ACES/releases/download/${ACES_CONFIG_RELEASE}"

# Pre-built configs published as release assets (for the chosen release).
CONFIG_ASSETS=(
  "reference-config-${ACES_CONFIG_RELEASE}_aces-v2.0_ocio-v2.5.ocio"
  "cg-config-${ACES_CONFIG_RELEASE}_aces-v2.0_ocio-v2.5.ocio"
  "studio-config-${ACES_CONFIG_RELEASE}_aces-v2.0_ocio-v2.5.ocio"
  "studio-config-all-views-${ACES_CONFIG_RELEASE}_aces-v2.0_ocio-v2.5.ocio"
  "studio-config-d60-views-${ACES_CONFIG_RELEASE}_aces-v2.0_ocio-v2.5.ocio"
)

FETCH_CONFIGS=1
for arg in "$@"; do
  case "$arg" in
    --no-configs) FETCH_CONFIGS=0 ;;
    -h|--help) sed -n '2,20p' "$0"; exit 0 ;;
    *) echo "unknown arg: $arg" >&2; exit 2 ;;
  esac
done

# ref/ lives next to this scripts/ dir, under sandbox/tocio/.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REF_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)/ref"
mkdir -p "${REF_DIR}/configs"

log()  { printf '\033[1;36m[fetch-ocio-ref]\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33m[fetch-ocio-ref]\033[0m %s\n' "$*" >&2; }

# Shallow-clone REF at the given ref, or update it if the dir already exists.
clone_or_update() {
  local url="$1" ref="$2" dest="$3"
  if [ -d "${dest}/.git" ]; then
    log "updating $(basename "$dest") -> ${ref}"
    git -C "${dest}" fetch --depth 1 origin "${ref}" --tags --force
    git -C "${dest}" checkout --force FETCH_HEAD
  else
    log "cloning $(basename "$dest") @ ${ref} (shallow)"
    # --branch accepts tags as well as branch names.
    git clone --depth 1 --branch "${ref}" "${url}" "${dest}"
  fi
}

clone_or_update "${OCIO_URL}" "${OCIO_REF}" "${REF_DIR}/OpenColorIO"
clone_or_update "${ACES_URL}" "${ACES_REF}" "${REF_DIR}/OpenColorIO-Config-ACES"

if [ "${FETCH_CONFIGS}" -eq 1 ]; then
  log "downloading pre-built .ocio configs (${ACES_CONFIG_RELEASE}) -> ref/configs/"
  for asset in "${CONFIG_ASSETS[@]}"; do
    out="${REF_DIR}/configs/${asset}"
    if curl -fSL --retry 3 -o "${out}" "${RELEASE_BASE}/${asset}"; then
      log "  ok: ${asset}"
    else
      warn "  failed: ${asset} (skipping)"
      rm -f "${out}"
    fi
  done
else
  log "skipping .ocio config download (--no-configs)"
fi

log "done. reference material is in: ${REF_DIR}"
log "  OpenColorIO library:     ${REF_DIR}/OpenColorIO        (${OCIO_REF})"
log "  ACES config repo:        ${REF_DIR}/OpenColorIO-Config-ACES (${ACES_REF})"
log "  pre-built .ocio configs: ${REF_DIR}/configs"
