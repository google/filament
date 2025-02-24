.. _documentation:

=======================
About the Documentation
=======================

boto's documentation uses the Sphinx__ documentation system, which in turn is
based on docutils__. The basic idea is that lightly-formatted plain-text
documentation is transformed into HTML, PDF, and any other output format.

__ http://sphinx.pocoo.org/
__ http://docutils.sf.net/

To actually build the documentation locally, you'll currently need to install
Sphinx -- ``easy_install Sphinx`` should do the trick.

Then, building the html is easy; just ``make html`` from the ``docs`` directory.

To get started contributing, you'll want to read the `ReStructuredText
Primer`__. After that, you'll want to read about the `Sphinx-specific markup`__
that's used to manage metadata, indexing, and cross-references.

__ http://sphinx.pocoo.org/rest.html
__ http://sphinx.pocoo.org/markup/

The main thing to keep in mind as you write and edit docs is that the more
semantic markup you can add the better. So::

    Import ``boto`` to your script...

Isn't nearly as helpful as::

    Add :mod:`boto` to your script...

This is because Sphinx will generate a proper link for the latter, which greatly
helps readers. There's basically no limit to the amount of useful markup you can
add.


The fabfile
-----------

There is a Fabric__ file that can be used to build and deploy the documentation
to a webserver that you ssh access to.

__ http://fabfile.org

To build and deploy::

    cd docs/
    fab deploy:remote_path='/var/www/folder/whatever' --hosts=user@host

This will get the latest code from subversion, add the revision number to the 
docs conf.py file, call ``make html`` to build the documentation, then it will
tarball it up and scp up to the host you specified and untarball it in the 
folder you specified creating a symbolic link from the untarballed versioned
folder to ``{remote_path}/boto-docs``.


