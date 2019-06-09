# Homebrew SOLO3 install - MAC OSX

1. [prepare](#prepare)
2. [download](#download)
3. [install](#install)
4. [verify](#verify)
5. [upgrade](#upgrade)

<a name="prepare"/>

## 1. Prepare

Install either the XCode development environment or a stand-alone version of the
XCode command line tools.  If you intend to do lots of Apple development and
want to use an IDE, then install XCode.

### Installing complete XCode

To install the full XCode package, get an Apple ID and register for the Apple App Store.

You will need to provide a credit card, so Apple can charge you if you actually buy anything.  
However, XCode is free.

From the App Store, install XCode.
Start XCode, open the preferences window, select the 'Downloads' tab, and 
install "Command Line Tools"

You may also need to run:

```
  xcode-select --install
```

### Installing stand-alone XCode command line tools

Alternatively, you can install the stand-alone XCode command line tools.

Download [Command Line Tools](http://developer.apple.com/downloads)

You will need to register for a free Apple id, no credit card is required.

### Install homebrew

The default location for homebrew is /usr/local. So you need write permission
to /usr/local to perform the install.

Run the following ruby script:

```
  /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  /usr/local/bin/brew update
```

## 2. Download

You need to download the brew formula from the solo3 repository.
This formula is used to perform the homebrew build.

Download [lrose-solo3.rb](https://github.com/NCAR/lrose-solo3)

<a name="install"/>

## 3. Build and install

Let us assume you have downloaded lrose-solo3.rb to your Downloads directory.

```
  cd ~/Downloads
  brew install lrose-solo3.rb
```

While homebrew is building, it creates log files so you can track the progress.

You can ignore the following message:

```
  Warning: lrose-solo3 dependency gcc was built with a different C++ standard
```

The location of the log files will be:

```
  ~/Library/Logs/Homebrew/lrose-solo3
```

You will see the following log files:

```
  00.options.out
  01.configure.cc
  01.configure
  02.make
  02.make.cc
```

You can watch the progress using:

```
  tail -f 01.configure
  tail -f 02.make
```

If the build is successful, the following binaries will be installed:

```
  /usr/local/bin/solo3
  /usr/local/bin/xltrs3
  /usr/local/bin/ddex3
  /usr/local/bin/nx_reblock3
```

These will be links that point to the actual files
in ```/usr/local/Cellar/lrose-solo3```.

See also: [Homebrew Notes](./homebrew_notes.md)

<a name="verify"/>

## 4. Verify the installation

Try the commands:
```
  /usr/local/bin/solo3 -h
```

<a name="upgrade"/>

## 5. Upgrade to a new version

When the time comes to upgrade, you will first need to uninstall the current version.

To find the name of the currently-installed lrose-solo3 package, run:

```
  brew list
```

Uninstall it as follows:

```
  brew uninstall lrose-solo3
```
Next, download the new version of [lrose-solo3.rb](https://github.com/NCAR/lrose-solo3/releases)

Choose from the appropriate distribution.

Then:

```
  cd ~/Downloads
  brew install lrose-solo3.rb
```

See [install](#install) for checking on the install.


