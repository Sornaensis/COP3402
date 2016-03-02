Group Members:
        Kyle Jones,
        Alex Hunt

Building:

######################################################################
##   _____ __  __ _____   ____  _____ _______       _   _ _______   ##
##  |_   _|  \/  |  __ \ / __ \|  __ \__   __|/\   | \ | |__   __|  ##
##    | | | \  / | |__) | |  | | |__) | | |  /  \  |  \| |  | |     ##
##    | | | |\/| |  ___/| |  | |  _  /  | | / /\ \ | . ` |  | |     ##
##   _| |_| |  | | |    | |__| | | \ \  | |/ ____ \| |\  |  | |     ##
##  |_____|_|  |_|_|     \____/|_|  \_\ |_/_/    \_\_| \_|  |_|     ## 
##                                                                  ##
######################################################################
| This project has two build options, one for the extended syntax that 
| is described in the accompanying word document, and the other for
| JUST parameter passing. By default the project will build with all
| special extensions ENABLED, as per the successful example included
| in ./success/*
|
| !!THIS EXAMPLE PROGRAM CANNOT WORK WITHOUT THE DEFAULT BUILD!!
|
| For testing ANY code that uses the following syntax:
|       return := <expression>
| the project must be RE-BUILT using the commands below the normal
| build procedure.

In your favorite $SHELL >

    $ make 
    $ ./pl0 {-t|-s|-m|-a|-v}

Rebuilding the project with ONLY the parameter extensions enabled:
    $ make clean
    $ make params-only
    $ ./pl0 {-t|-s|-m|-a|-v}

Running
    $ make clean
between builds is necessary to build the project with different settings.
