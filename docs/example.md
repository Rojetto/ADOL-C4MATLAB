Example
=======

This example demonstrates how to use the ADOL-C4MAT toolbox.

Consider the function

```math
y = x_1 \sin(x_1 + x_2^2).
```

At first this function needs to be expressed using C syntax where independent variables have to be `x[0]` ... `x[n-1]` dependent variables have to be `y[0]` ... `y[m-1]`:

    y[0] = x[0]*sin(x[0] + pow(x[1], 2));

This short code snipped is stored in `XSinXY.m`.

Next, the tolboxes settings file must be loaded:

    SettingsFile    = '../madSettings';
        
    % load settings
    [filepath, name] = fileparts([SettingsFile, '.m']);
    addpath(filepath);
    Settings = eval(name);
    rmpath(filepath);

Now the tapes may be generated. Therefore the number of dependent and independent variables has to be known. For tape generation the function [madTapeCreate](taping.md#madTapeCreate) is used:

    n = 2;      % number of independent variables
    m = 1;      % number of dependent variables
    keep = 0;   % prepare for an immediate call of the reverse mode (see the ADOL-C manual)

    % generate the tape
    TapeId = madTapeCreate(n, m, keep, 'XSinXY.m', Settings);

Since the tape is generated and already opened using the Id `TapeId`, this can be used for a simple function evaluation at the point $`(3, 4)`$:

    >> X = [3 4]';
    >> Y = madFunction(TapeId, X)

    Y =
       0.449631628988857

In the same manner the Jacobian or Hessian may be computed:

    >> dY = madJacobian(TapeId, X)

    dY = 
       3.115991064222960  23.728910836480061

    >> ddy = madHessian(TapeId, X)

    ddY = 
       1.527777607384481   4.312583913582498
       4.312583913582498 -22.844196546166835

When the tape is not any longer required it should be closed using [madTapeClose](taping.md#madTapeClose):

    madTapeClose(TapeId);