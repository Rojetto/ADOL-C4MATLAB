Drivers
=======

This math is inline $`a^2+b^2=c^2`$.

This is on a separate line
```math
a^2+b^2=c^2
```


Basic Drivers
-------------

### madForward


Calculates the scaled Taylor coefficients of the image path.

#### Synopsis

	Z = madForward(TapeId, d, keep, X)
	
#### Description

This function calculates the scaled Taylor coefficients :math:`z_i` :math:`(i=0,\\ldots,d)` of the image path 

```math
z(t) = F(x(t)) = z_0 + z_1 t + z_2 t^2 + \\ldots + z_d t^d + \\mathcal{O}(t^d)
```

of the function $`F`$ represented by the tape with the tape number `TapeId`. The $`n$ \times d`$ matrix $`X`$ contains the Taylor coefficients $`x_i`$ ($`i=0,\ldots,d`$) of the path 

```math
	x(t) = x_0 + x_1 t x_2 t^2 + \ldots + x_d t^d + \mathcal{O}(t^d)
```

with `n` the number of independent variables of the function and `d` the number of derivatives of :math:`x(t)`. The flag `keep` determines how many derivatives are internally stored for the use of the reverse mode. The following must hold: :math:`1 \leq \mathtt{keep} \leq \mathtt{d}+1`. The function returns a :math:`\mathtt{m} \times \mathtt{d}+1` matrix containing the Taylor coefficients :math:`z_i` (`i=0,\ldots,d`) of the image path with `m` the number of dependent variables.

Keep in mind that the Taylor coefficients of the paths are defined as follows:

.. math:
	
	x_k = \frac{1}{k!}\frac{\partial^k}{\partial t^k}x(t), \qquad z_k = \frac{1}{k!}\frac{\partial^k}{\partial t^k}z(t), \qquad k = 0, \ldots, d.


Example
*******
	
It is assumed that the function

.. math:
	
	{y} = \begin{pmatrix}
		x_1^2x_2 + x_1\cos(x_2)\\
		x_2^2\sin(x_1) + x_2x_1^2
	\end{pmatrix}

is represented by the tape with the number `TapeId` . If one has :math:`x_1(0) = 3, \dot x_1(0) = -3, \ddot x_1(0) = -6` and :math:`x_2(0) = 4, \dot x_1(0) = 4, \ddot x_1(0) = 8` and one wants to calculate :math:`z_0, z_1, z_2` one has to do the following:

::

	>> X = [3 4; -3 4; -3 4]';
	>> Z = madForward(TapeId, 2, 2, X)
	
	Z = 
		34.0391 -24.9574 -54.3516
		38.2579  16.0355  67.1720
