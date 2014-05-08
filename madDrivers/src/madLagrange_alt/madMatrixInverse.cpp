/* Inverse of a n by n matrix */
#include<stdio.h>
#include<conio.h>
#include<math.h>
#include<stdlib.h>

int power(int number, int index);

void main()
{
	void arg(int *,int *, int *,int ,int );
	int det(int *,int *);
	int a[10][10],b[10][10],c[10][10],n,i,j,m,d;
	//clrscr();
	printf("Enter the order of the matrix");
	scanf("%d",&n);
	printf("Enter the matrix");
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
			scanf("%d",&a[i][j]);
	}

	if(n==2)
	{
		c[0][0]=a[1][1];
		c[1][1]=a[0][0];
		c[0][1]=-a[0][1];
		c[1][0]=-a[1][0];
		d=a[0][0]*a[1][1]-a[0][1]*a[1][0];
		printf("Determinant is:%d\n",d);
		if(d==0)
		{
			getch();
			exit(d-'0');
		}

		for(i=0;i<n;i++)
		{
			printf("\n");
			for(j=0;j<n;j++)
				printf(" %f",c[i][j]/(float)d);

		}
	}
	else
	{
		m=n;
		for(i=0;i<m;i++)
		{
			for(j=0;j<m;j++)
			{
				n=m;
				arg(&a[0][0],&b[0][0],&n,i,j);
				c[j][i]=power(-1,(i+j))*det(&b[0][0],&n);
			}
		}
		n=m;
		d=det(&a[0][0],&n);
		printf("Determinant is :%d\n",d);
		if(d==0)
		{
			printf("INVERSE DOES NOT EXIST");
			getch();
			exit(d-'0');
		}
		for(i=0;i<m;i++)
		{
			printf("\n");
			for(j=0;j<m;j++)
				printf(" %f",c[i][j]/(float)d);
		}
	} getch();
}

void arg(int *a,int *b,int *n,int x,int y)
{
	int k,l,i,j;
	for(i=0,k=0;i<*n;i++,k++)
	{
		for(j=0,l=0;j<*n;j++,l++)
		{
			if(i==x)
				i++;
			if(j==y)
				j++;
			*(b+10*k+l)=*(a+10*i+j);

		}
	}
	*n=*n-1;
}

int det(int *p,int *n)
{
	int d[10][10],i,j,m,sum=0;
	m=*n;
	if(*n==2)
		return(*p**(p+11)-*(p+1)**(p+10));
	for(i=0,j=0;j<m;j++)
	{
		*n=m;
		arg(p,&d[0][0],n,i,j);
		sum=sum+*(p+10*i+j)*power(-1,(i+j))*det(&d[0][0],n);
	}

	return(sum);
}

int power(int number, int index) 
{
	if (index == 0) 
	{
		return 1;
	}
	int num = number;
	for (int i = 1; i < index; i++) 
	{
		number = number * num;
	}
	return number;
}