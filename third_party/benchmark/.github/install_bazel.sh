if ! bazel version; then
  arch=$(uname -m)
  if [ "$arch" == "aarch64" ]; then
    arch="arm64"
  fi
  echo "Downloading $arch Bazel binary from GitHub releases."
  curl -L -o $HOME/bin/bazel --create-dirs "https://github.com/bazelbuild/bazel/releases/download/8.2.0/bazel-8.2.0-linux-$arch"
  chmod +x $HOME/bin/bazel
else
  # Bazel is installed for the correct architecture
  exit 0
fi
