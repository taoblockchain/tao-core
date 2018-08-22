Tao Core integration/staging tree
=====================================

https://tao.network

What is the Tao Blockchain?
---------------------------

The Tao Blockchain is an experimental smart contract platform protocol that enables 
instant payments to anyone, anywhere in the world in a private, secure manner. 
Tao uses peer-to-peer blockchain technology developed by Bitcoin to operate
with no central authority: managing transactions, execution of contracts, and 
issuing money are carried out collectively by the network. Tao Core is the name of 
open source software which enables the use of this protocol.

For more information, see https://tao.network/index.html, or read the
[original whitepaper](https://tao.network/Welcome%20to%20Tao%20v1.4.pdf).

Building the wallet
-------------------
A simple guide for building the linux wallet is located [here](/doc/simple-build-unix.md)

Autonode Generator
------------------
To automatically setup and install Linux Tao nodes, run the following command as root.

wget -O taoNode.sh https://raw.githubusercontent.com/taoblockchain/tao-core/master/contrib/autonode.sh ; sudo bash taoNode.sh

License
-------

Tao Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/taoblockchain/tao-core/tags) are created
regularly to indicate new official, stable release versions of Tao Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

The developer [mailing list](https://lists.linuxfoundation.org/mailman/listinfo/bitcoin-dev)
should be used to discuss complicated or controversial changes before working
on a patch set.

Developer Slack can be found at http://taoblockchain.slack.com.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.
