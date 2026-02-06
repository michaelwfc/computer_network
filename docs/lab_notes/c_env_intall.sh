# Check all at once
echo "=== G++ ===" && g++ --version
echo "=== Clang-tidy ===" && clang-tidy --version 2>/dev/null || echo "Not installed"
echo "=== Clang-format ===" && clang-format --version 2>/dev/null || echo "Not installed"
echo "=== CMake ===" && cmake --version
echo "=== Git ===" && git --version
echo "=== Mininet ===" && mn --version 2>/dev/null || echo "Not installed"
echo "=== Tcpdump ===" && tcpdump --version 2>&1 | head -1
echo "=== Wireshark ===" && wireshark --version 2>/dev/null | head -1 || echo "Not installed"

# Install Missing Packages
# Update package list first
sudo apt update

# Install build-essential (includes g++, gcc, make, etc.)
# sudo apt install -y build-essential

# Install all required packages
sudo apt install -y \
    build-essential \
    gdb \
    clang \
    clang-tidy \
    clang-format \
    cppcheck \
    gcc-doc \
    pkg-config \
    glibc-doc \
    cmake \
    libpcap-dev \
    git \
    iptables \
    mininet \
    tcpdump \
    tshark \
    telnet \
    wireshark \
    socat \
    netcat-openbsd \
    coreutils \
    bash \
    doxygen \
    graphviz \
    python3-pip

# Verify installations
echo "=== Verification ==="
g++ --version | head -1
clang-tidy --version
cmake --version | head -1
git --version


# For Specific Versions (if needed)
# If you need specific older versions like g++ 8.x:
# Add repository for older versions (if needed)
# sudo apt install -y software-properties-common

# # Install specific g++ version
# sudo apt install -y g++-8

# # Set default g++ version (if multiple installed)
# sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 8
# sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 8

# # Choose which version to use
# sudo update-alternatives --config g++


# Check C++ Standard Support
# Check which C++ standards are supported
g++ -v 2>&1 | grep "gcc version"

# Test C++23 compilation
echo 'int main() { return 0; }' > test.cpp
g++ -std=c++23 test.cpp -o test && echo "C++23 supported" || echo "C++23 not supported"
g++ -std=c++20 test.cpp -o test && echo "C++20 supported" || echo "C++20 not supported"
g++ -std=c++17 test.cpp -o test && echo "C++17 supported" || echo "C++17 not supported"
rm test.cpp test