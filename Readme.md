ADOL-C4MATLAB
=============

ADOLC-4MATLAB provides an interface to the open source algorithmic differentiation toolbox [ADOL-C](https://gitlab.com/adol-c/adol-c) for MATLAB&trade; and Octave.

## Target

The toolbox is intended for students and researches in control theory. Many algorithms in nonlinear control require the computation of derivatives like Jacobians, Hessians or certain Lie derivatives. Especially for complex systems, the symbolic computation can result in a time and memory consuming task as the derivative order increases. This effect may be avoided when algorithmic differentiation is used instead of the symbolic computation of derivatives.
The open source algorithmic differentiation toolbox ADOL-C is designed for the computation of first and higher order derivatives of vector functions that predestines it for tasks in control engineering. It is written in C/C++ and can be easily connected to MATLAB using so called MEX functions. This is where ADOL-C4MATLAB comes in.

## Functionality of the Interface

Functions that are going to be used for differentiation operations first are passed to an automated procedure to build a MEX function, which, when executed, generates so called tapes. This is a special data set containing all necessary information of a considered function which then ADOL-C uses for derivative computation later.
Furthermore, the interface provides a series of ready to use MEX functions accessing the tapes for their operations, e.g., to get the gradient, Jacobian or Hessian of a function or various Lie derivatives along vector fields. Part of this functions are MATLAB MEX wrappers around the corresponding ADOL-C functions. Beside this basic functionality there is also a more sophisticated and control engineering related part. This contains the direct computation of gains for controllers as the exact input-output linearization or observers such as the extended Luenberger observer.
The structure of the interface also allows an easy extension by user defined functions.