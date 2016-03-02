Building
--------

In your favorite $SHELL >

    $ make 
    $ ./pl0 [FILE] {-t|-s|-m|-a|-v}

Rebuilding the project with ONLY the parameter extensions enabled:
    $ make clean
    $ make params-only
    $ ./pl0 [FILE] {-t|-s|-m|-a|-v}

Running
    $ make clean
between builds is necessary to build the project with different settings.

Invocation
----------

    $ ./pl0 [FILE] {-t|-s|-m|-a|-v}

Omitting filename will use './input.txt' by default
