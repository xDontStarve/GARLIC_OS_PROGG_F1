/*------------------------------------------------------------------------------

	"PRTF.c" : programa de test de usuario progG;
				(versi�n 1.0)
	
	Imprime n�meros Long Long

------------------------------------------------------------------------------*/

#include <GARLIC_API.h>			/* definici�n de las funciones API de GARLIC */
unsigned int maximo_comun_divisor(int, int);			// Funcion auxiliar de lcm (calcula el m�nimo com�n divisor).
int minimo_comun_multiple(int, int);		

// Funcion que calcula el m�nimo com�n m�ltiple de 50 parejas de n�meros aleatorios desde el 1 hasta 1000*(arg+1)
int _start(int arg) 
{
	int maxNum=1000*(arg+1), numParejas=0, num1, num2;
	while (numParejas<50)
	{
		unsigned int quo=0, mod=0;
		GARLIC_divmod(GARLIC_random(), maxNum+1, &quo, &mod);
		num1=mod;	//Rango 1 a 1000*(arg+1)
		GARLIC_divmod(GARLIC_random(), maxNum+1, &quo, &mod);
		num2=mod;
		GARLIC_printf("(%d) ", numParejas);
		GARLIC_printf("El minimo comun divisor de %d y %d es: ", num1, num2);
		GARLIC_printf("%d\n", minimo_comun_multiple(num1, num2));
		numParejas++;
	}
	return 0;
}

unsigned int maximo_comun_divisor(int a, int b) {
    while (b != 0) {
        unsigned int temp = b, quo=0, mod=0;
		GARLIC_divmod(a, b, &quo, &mod);
        b = mod;
        a = temp;
    }
    return a;
}

int minimo_comun_multiple(int a, int b) {
	unsigned int resultado=0, mod=0, mult=a*b, gcd;
	gcd=maximo_comun_divisor(a, b);
	GARLIC_divmod(mult, gcd, &resultado, &mod);
    return resultado;
}
