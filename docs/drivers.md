Tape Usage
==========

- [Basic Drivers](#basic-drivers)
	- [madForward](#madforward)
	- [madFunction](#madfunction)
	- [madGradient](#madgradient)
	- [madHessian](#madhessian)
	- [madJacobian](#madjacobian)
	- [madLieBracket](#madliebracket)
	- [madLieCovector](#madliecovector)
	- [madLieGradient](#madliegradient)
	- [madLieMixedDerivative](#madliemixedderivative)
	- [madLieScalar](#madliescalar)
	- [madReverse](#madreverse)
- [Advanced Drivers](#advanced-drivers)
	- [madCompTorqueControl](#madcomptorquecontrol)
	- [madCompTorqueLagrange](#madcomptorquelagrange)
	- [madExtLuenObs](#madextluenobs)
	- [madFeedbackLin](#madfeedbacklin)
	- [madHighGainObs](#madhighgainobs)
	- [madLagrange](#madlagrange)
	- [madLagrangePartLin](#madlagrangepartlin)
	- [madPartLin](#madpartlin)

## Basic Drivers

### madForward

Calculates the scaled Taylor coefficients of the image path.

#### Synopsis

	Z = madForward(TapeId, d, keep, X) 
	
#### Description

This function calculates the scaled Taylor coefficients $`z_i`$ $`(i=0,\ldots,d)`$ of the image path 

```math
z(t) = F(x(t)) = z_0 + z_1 t + z_2 t^2 + \ldots + z_d t^d + \mathcal{O}(t^d)
```

of the function $`F`$ represented by the tape with the tape number `TapeId`. The $`n \times d`$ matrix $`X`$ contains the Taylor coefficients $`x_i`$ ($`i=0,\ldots,d`$) of the path 

```math
x(t) = x_0 + x_1 t x_2 t^2 + \ldots + x_d t^d + \mathcal{O}(t^d)
```

with `n` the number of independent variables of the function and `d` the number of derivatives of $`x(t)`$. The flag `keep` determines how many derivatives are internally stored for the use of the reverse mode. The following must hold: $`1 \leq \mathtt{keep} \leq \mathtt{d}+1`$. The function returns a $`\mathtt{m} \times \mathtt{d}+1`$ matrix containing the Taylor coefficients $`z_i`$ ($`i=0,\ldots,d`$) of the image path with $`m`$ the number of dependent variables.

Keep in mind that the Taylor coefficients of the paths are defined as follows:

```math
x_k = \frac{1}{k!}\frac{\partial^k}{\partial t^k}x(t), \qquad z_k = \frac{1}{k!}\frac{\partial^k}{\partial t^k}z(t), \qquad k = 0, \ldots, d.
```

#### Example
	
It is assumed that the function

```math
y = \begin{pmatrix}
	x_1^2x_2 + x_1\cos(x_2)\\
	x_2^2\sin(x_1) + x_2x_1^2
\end{pmatrix}
```

is represented by the tape with the number `TapeId` . If one has $`x_1(0) = 3`$, $`\dot x_1(0) = -3`$, $`\ddot x_1(0) = -6`$ and $`x_2(0) = 4`$, $`\dot x_1(0) = 4`$, $`\ddot x_1(0) = 8`$ and one wants to calculate $`z_0, z_1, z_2`$ one has to do the following:

	>> X = [3 4; -3 4; -3 4]';
	>> Z = madForward(TapeId, 2, 2, X)
	
	Z = 
		34.0391 -24.9574 -54.3516
		38.2579  16.0355  67.1720



### madFunction

Calculates the value of a function.

#### Synopsis

	Y = madFunction(TapeId, X)

#### Description

`madFunction` calculates the value of a function tape which is referenced by the number `TapeId`. The point at which the function is evaluated is given by the column vector `X`. The value of the function is returned in the column vector `Y`.

#### Example

It is assumed that the function

```math
y = \begin{pmatrix} x_1^2x_2 + x_1\cos(x_2) \\ x_2^2\sin(x_1) + x_2x_1^2 \end{pmatrix}
```
is represented by the tape with the number `TapeId`. Then the value of the function at the point $`(1,2)^T`$ is calculated by the following commands:

	>> X = [1; 2];
	>> Y = madFunction(TapedId, X)

	Y =
	    1.5839
	    5.3659




### madGradient

Calculates the Gradient of a scalar field.

#### Synopsis

	G = madGradient(TapeId, X)
	
#### Description

`madGradient` calculates the gradient of a scalar field represented by a tape with the number `TapeId`. The point of evaluation of the gradient is given by the column vector `X`. The gradient is returned in the column vector `G`.

#### Example

Assume that the function 

```math
y = x_1^2 x_2 + x_1 \cos(x_2)
``` 
is represented by the tape with the number `TapeId`. Then the value of the gradient at the point $`(3, 4)^T`$ is calculated by the following commands:

	>> X = [3; 4];
	>> G = madGradient (TapeId , X)
	
	G =
	    23.3464
	    11.2704




### madHessian

Calculates the Hessian of a function.

#### Synopsis

	H = madHessian(TapeId, X)
	
#### Description

`madHessian` calculates the Hessian of a function represented by a tape with the number `TapeId`. The point of evaluation of the Hessian is given by the column vector `X`. The Hessian is returned in the matrix `H`.

#### Example

Assume that the function 

```math
y = x_1^2 x_2 + x_1 \cos(x_2)
``` 

is represented by the tape with the number `TapeId`. Then the value of the Hessian at the point $`(3, 4)^T`$ is calculated by the following commands:

	>> X = [3; 4];
	>> H = madHessian (TapeId , X)

	H =
	    8.0000 6.7568
	    6.7568 1.9609




### madJacobian

Calculates the Jacobian of a function.

#### Synopsis

	J = madJacobian(TapeId, X)
	
#### Description

\c madJacobian calculates the Jacobian of a function represented by a tape with the number `TapeId`. The point of evaluation of the Jacobian is given by the column vector `X`. The Jacobian is returned in the matrix `J`.

#### Example

Assume that the function 
```math
{y} = \begin{pmatrix} x_1^2x_2 + x_1\cos(x_2) \\ x_2^2\sin(x_1) + x_2x_1^2 \end{pmatrix}
```
is represented by the tape with the number `TapeId`. Then the value of the Hessian at the point $`(3, 4)^T`$ is calculated by the following commands:

	>> X = [3; 4];
	>> J = madJacobian(TapeId , X´)
	
	J =
	    23.3464 11.2704
	     8.1601 10.1290




### madLieBracket




### madLieCovector




### madLieDerivative




### madLieGradient




### madLieMixedDerivative




### madLieScalar




### madReverse





## Advanced Drivers


### madCompTorqueControl



### madCompTorqueLagrange



### madExtLuenObs



### madFeedbackLin



### madHighGainObs

Calculates the observer gain for a High-Gain observer.

#### Synopsis

for SISO systems:

	kHG = madHighGainObs(TapeIdF, TapeIdH, X, k)

for MIMO systems:

	KHG = madHighGainObs(TapeIdF, TapeIdH, X, K, kappa)

#### Description

#### Examples

##### Lorenz System

Consider that the fector field $`f(x)`$ and the scalar field $`h(x)`$ of the Lorenz system

```math
\dot{x} = f(x) = \begin{pmatrix}
	\sigma(x_2 - x_1) \\ rx_1 - x_2 -x_1x_3 \\ -bx_3 - x_1x_2
\end{pmatrix}
```

with the output

```math
	y = h(x) = \arctan x_3
```

where $`\sigma = 10`$, $`b = 8/3`$, $`r = 28`$ are represented by the tapes with the numbers `TapeIdF` and `TapeIdH`.

For simplicity the observer dynamics is chosen such that all poles are $`-10`$. This results in $`k=(30, 300, 1000)^T`$. The initial observer state is chosen to be $`\hat x_0 = (1, 1, 1)^T`$ while the true initial state is $`x_0 = (10, 10, 10)^T`$.


	k = [30; 300; 1000];
	X = [1; 1; 1];
	kHG = madHighGainObs(TapeIdF, TapeIdH, X, k)
	
	kHG = ...



### madLagrange



### madLagrangePartLin



### madPartLin

#### Synopsis
#### Description
#### Example
		