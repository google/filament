if ! bazel version; then
  arch=$(uname -m)
  if [ "$arch" == "aarch64" ]; then
    arch="arm64"
  fi
  echo "Downloading $arch Bazel binary from GitHub releases."
  curl -L -o $HOME/bin/bazel --create-dirs "https://github.com/bazelbuild/bazel/releases/download/7.1.1/bazel-7.1.1-linux-$arch"
  chmod +x $HOME/bin/bazel
else
  # Bazel is installed for the correct architecture
  exit 0
fi
