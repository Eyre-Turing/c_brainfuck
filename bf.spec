Name:           bf
Version:        1.0
Release:        1%{?dist}
Summary:        A compiler and interpreter for BF language

License:        GPLv3
URL:            https://github.com/Eyre-Turing/c_brainfuck
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc
Requires:       gcc

%description
A compiler and interpreter for BF language

%prep
%setup -q


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
make
cp bf $RPM_BUILD_ROOT/usr/bin/


%files
%doc
/usr/bin/bf


%changelog
