if ! bazel version; then
  arch=$(uname -m)
  if [ "$arch" == "aarch64" ]; then
    arch="arm64"
  fi
  echo "Installing wget and downloading $arch Bazel binary from GitHub releases."
  yum install -y wget
  wget "https://github.com/bazelbuild/bazel/releases/download/6.3.0/bazel-6.3.0-linux-$arch" -O /usr/local/bin/bazel
  chmod +x /usr/local/bin/bazel
else
  # bazel is installed for the correct architecture
  exit 0
fi
