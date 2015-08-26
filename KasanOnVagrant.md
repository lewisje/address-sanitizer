# Running AddressSanitizerForKernel on Vagrant #

Install [VirtualBox](https://www.virtualbox.org/) and [Vagrant](https://www.vagrantup.com/).

Clone this repository and start the VM:
```
# run on host
svn checkout http://address-sanitizer.googlecode.com/svn/trunk/vagrant_kasan
cd vagrant_kasan
vagrant up
vagrant ssh
```

Build and install modified GCC and build kernel with it:
```
# run on guest
/vagrant/build.sh get_gcc
/vagrant/build.sh make_gcc
/vagrant/build.sh get_kasan
/vagrant/build.sh make_kasan
```

Install the kernel and reboot:
```
# run on guest
sudo dpkg -i linux-image-*_amd64.deb
sudo shutdown -r now
```

Run the tests and check result:
```
# run on guest as root
echo asan_run_tests > /proc/kasan_tests
echo asan_run_stack > /proc/kasan_tests
cat /proc/kasan_stats
dmesg
```