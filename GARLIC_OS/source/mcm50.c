/*------------------------------------------------------------------------------

	"PRTF.c" : programa de test de usuario progG;
				(versión 1.0)
	
	Imprime números Long Long

------------------------------------------------------------------------------*/

#include <GARLIC_API.h>			/* definición de las funciones API de GARLIC */
unsigned int maximo_comun_divisor(int, int);			// Funcion auxiliar de lcm (calcula el mínimo común divisor).
int minimo_comun_multiple(int, int);		

// Funcion que calcula el mínimo común múltiple de 50 parejas de números aleatorios desde el 1 hasta 1000*(arg+1)
// Como GARLIC_divmod devuelve un int, no tiene sentido hacer las operaciones con Long long, ya que ocupara mas espacio
int mcm50(int arg) 
{
	int numParejas=0, num1, num2, randomIntNumber;
	unsigned int maxNum=1000*(arg+1)+1;
	long long randomLongNumber;
	while (numParejas<50)
	{
		unsigned int mod=0;
		long long quo=0;
		
		randomIntNumber=GARLIC_random();
		randomLongNumber=(long long) randomIntNumber;
		GARLIC_divmodL(&randomLongNumber, &maxNum, &quo, &mod);
		num1=mod;	//Rango 1 a 1000*(arg+1)
		
		randomIntNumber=GARLIC_random();
		randomLongNumber=(long long) randomIntNumber;
		GARLIC_divmodL(&randomLongNumber, &maxNum, &quo, &mod);
		num2=mod;
		
		GARLIC_printf("(%d) ", numParejas);
		long long longNum1, longNum2;
		longNum1=(long long) num1; longNum2=(long long) num2;
		GARLIC_printf("El minimo comun multiple de %l y %l es: ", &longNum1, &longNum2);
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