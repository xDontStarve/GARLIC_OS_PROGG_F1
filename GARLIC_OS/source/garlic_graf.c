/*------------------------------------------------------------------------------

	"garlic_graf.c" : fase 1 / programador G

	Funciones de gesti n de las ventanas de texto (gr ficas), para GARLIC 1.0

------------------------------------------------------------------------------*/
#include <nds.h>

#include <garlic_system.h>	// definici n de funciones y variables de sistema
#include <garlic_font.h>	// definici n gr fica de caracteres


/* definiciones para realizar c lculos relativos a la posici n de los caracteres
	dentro de las ventanas gr ficas, que pueden ser 4 o 16 */
#define NVENT	4				// n mero de ventanas totales
#define PPART	2				// n mero de ventanas horizontales o verticales
								// (particiones de pantalla)
#define VCOLS	32				// columnas y filas de cualquier ventana
#define VFILS	24
#define PCOLS	VCOLS * PPART	// n mero de columnas totales (en pantalla)
#define PFILS	VFILS * PPART	// n mero de filas totales (en pantalla)
int fondo2, fondo3, map2ptr;
/* _gg_generarMarco: dibuja el marco de la ventana que se indica por par metro*/
void _gg_generarMarco(int v)
{
	if (v>PPART*PPART) return;
	//Posicionar en la posición correcta del mapa
	//fila de ventana (0-(PPART-1)) * tamaño de una fila de ventanas (PCOLS*VFILS) + Desplazamiento horizontal (columna de ventana * tam. de 1 ventana).
	u16 * windowPointer = bgGetMapPtr(fondo3)+(((v/PPART)*PCOLS*VFILS)+(v%PPART)*VCOLS);
	for (int i=1; i<VCOLS-1; i++){
		*(windowPointer+i)=99;										//Norte: base + desplazamiento columnas
		*(windowPointer+(i+(VFILS-1)*PCOLS))=97;					//Sud: base + posiciones hasta ultima fila + desplazamiento columnas
		if (i<24)
		{	//Hay menos filas que columnas.
		*(windowPointer+(i*PCOLS))=96;								//Oeste: base + i*total de columnas (mapa[i][0]) == desplazar i filas hacia abajo desde la base
		*(windowPointer+(i*PCOLS)+VCOLS-1)=98;						//Este: Oeste + Columnas hasta la ultima columna de la ventana
		}
	}
	*windowPointer=103;												//Noroeste
	*(windowPointer+VCOLS-1)=102;									//Noreste
	*(windowPointer+((VFILS-1)*PCOLS))=100;							//Sudoeste
	*(windowPointer+((VFILS-1)*PCOLS+VCOLS-1))=101;					//Sudeste
	
}

/* _gg_iniGraf: inicializa el procesador gr fico A para GARLIC 1.0 */
void _gg_iniGrafA()
{

	videoSetMode(MODE_5_2D);				//Mode 2D para fondos
	lcdMainOnTop();
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	//bgInit(int layer, BgType type, BgSize size, int mapBase, int tileBase), devuelve el  ndice de fondo.
	//Por defecto los mapas de la NDS son de 32x32 baldosas, y ocupan 2 Kbytes. La pantalla es de 256x192 pixeles.
	fondo2 = bgInit(2, BgType_ExRotation, BgSize_ER_512x512, 4, 2);
	fondo3 = bgInit(3, BgType_ExRotation, BgSize_ER_512x512, 8, 2);
	map2ptr = (int) bgGetMapPtr(fondo2);
		/* MapBase indica el indice del mapa, en nuestro caso cada mapa ocupa 8192Bytes, ya que en 512x512 pixeles hay
		   64x64 baldosas y cada baldosa ocupa 2Bytes en memoria.
		   Podemos reutilizar TileBase porque son las mismas baldosas para cualquier fondo.
		   
		   En esta organizacion de virtual VRAM quedaria una estructura asi:
		   Base fondo0 = 0x06000000, no se usa, usamos la dir. para guardar tiles descomprimidos.
		   Base fondo1 = 0x06002000
		   Base fondo2 = 0x06004000
		   Base fondo3 = 0x06006000, no se usa
		   Base Baldosas = 0x06008000
		*/
	//Rango de prioridad = 0-3, menor* significa más prioridad
	bgSetPriority(fondo2, 2);
	bgSetPriority(fondo3, 1);
	//		  data to decompress, destination, decompress type.
	decompress(garlic_fontTiles, bgGetGfxPtr(fondo3), LZ77Vram); 
	/*Hay que indicar el sitio donde dejar las gráficas, en este caso como los dos fondos comparten tiles,
	  Los podemos dejar en la zona de memoria para tiles del fondo3.
	  BG_PALETTE=((u16*)0x05000000)*/
	dmaCopy(garlic_fontPal, BG_PALETTE, sizeof(garlic_fontPal));
	for (int i=0 ; i< NVENT ; i++)
	{
		_gg_generarMarco(i);
	}
	//	para hacer zoom de 50%, hay que dividir los 512x512 entre 2 para caber en la pantalla de NDS (zoom x2).
	bgSetScale(fondo2, 512, 512);// 2 -> 2^1 = 2: 0...10,0000 0000 = 0d512
	bgSetScale(fondo3, 512, 512);// 24b Parte Entera-^ ^-8 bits de decimal
	
	bgUpdate();
}

/*
	_gg_long2str: Transforma un número long long de C a una string acabado en \0

	Parámetros:
		int pointed: 0: No poner puntos cada 3 dígitos, 1: Poner puntos cada 3 dígitos.
		unsigned int numPtr: Puntero (int) a un número long long de C
		char* str:	Buffer donde ir guardando el resultado, como máximo se guardará 19 dígitos.
		
	Resultado -> Mensaje guardado en la string pasado por parámetro.
	
	Observación: Un número long tiene rango -9,223,372,036,854,775,808 - 9,223,372,036,854,775,807
				No se hace control de tamaño máximo de string en esta función.
*/

void _gg_long2str(int pointed, unsigned int numPtr, char* str)
{
	int i=0, j=0, negativo=0;
	long long* num=(long long*)numPtr;		//Convertir el uint a puntero a long
	long long longNum=*num;					//Coger el valor del long.
	char temp;
	int digitCount=0;
	
	if (longNum<0)
	{
		negativo=1;
		longNum=-longNum;						//Tratar el nÃºmero en positivo, luego aÃ±adir el signo de negativo.
	}
	do {
		if (pointed && (digitCount > 0) && (digitCount%3==0)) {
			str[i++]='.';						//Si pointed es 1 se aÃ±aden periodos cada 3 dÃ­gitos.
		}
		str[i++]=(longNum % 10) + '0';		//Sumar '0' para poner el offset de los nÃºmeros en formato ASCII
		longNum/=10;							//Dividir el long de 10 en 10 para coger los valores. 
		digitCount++;
	} while (longNum > 0);
	if (negativo) str[i++]='-';		
	str[i]='\0';								//AÃ±adir centinela.
	
	//Invertir el orden de la string para representarlo correctamente.
	//Si se quiere guardar el número en formato string directamente en el orden correcto,
	//se tendria que calcular la longitud del num long long, teniendo en cuenta el signo y los puntos, por lo que resulta mas costoso.
	for (j=0; j<i/2; j++) {
		temp=str[j];
		str[j]=str[i-j-1];
		str[i-j-1]=temp;
	}
}



/* _gg_procesarFormato: copia los caracteres del string de formato sobre el
					  string resultante, pero identifica los c digos de formato
					  precedidos por '%' e inserta la representaci n ASCII de
					  los valores indicados por par metro.
	Par metros:
		formato	->	string con c digos de formato (ver descripci n _gg_escribir);
		val1, val2	->	valores a transcribir, sean n mero de c digo ASCII (%c),
					un n mero natural (%d, %x) o un puntero a string (%s);
		resultado	->	mensaje resultante.
	Observaci n:
		Se supone que el string resultante tiene reservado espacio de memoria
		suficiente para albergar todo el mensaje, incluyendo los caracteres
		literales del formato y la transcripci n a c digo ASCII de los valores.
*/
void _gg_procesarFormato(char *formato, unsigned int val1, unsigned int val2,
																char *resultado)
{
	//char=1Bytes, %d = 1.32, %x = hexa
	int message_length=0, currentParamNumber=1;
	//Hace falta un contador para saber qué posición de formato coger el carácter.
	int i=0;
	char *token=formato, *currentChar;
	/*Mientras no sea el centinela ni se ha pasado de 3 líneas(sin contar \0), seguir procesando
		Las funciones de sistema para convertir números usa los parámetros de la siguiente manera:
			- Guarda el número decimal (0d) en formato string en el puntero pasado en el primer parámetro (acabado en \0)
			- El segundo parámetro es el número que bytes (char) que tiene el parámetro 1
			- El tercer parámetro es la variable con el número natural a transformar
			* Devuelve 0 si se ha podido transformar.
	*/
	while ((token[i]!='\0') && message_length<3*VCOLS)				//+1: que escribir el centinela
	{
		if (token[i]=='%')
		{
			if (currentParamNumber>2)
			{
				//Si ha superado el límite de parámetros, imprimir tal cual el formato
				//Puede que supere el límite de 3 líneas pero se ha guardado 3*VCOLS+1 espacios en la array, por lo cual el último carácter se sustituiría por \0 en escribirlinea
				resultado[message_length++]='%';
				resultado[message_length++]=token[i+1];
				i +=2;													//Se suma 2 porque el formato ocupa 2 ASCII (%c)
			}else
			{
				switch (token[i+1])
				{
					case '%':											//Copiar % al formato final
					resultado[message_length]='%';
					message_length++;
					i +=2;												//Se suma 2 porque el formato ocupa 2 ASCII (%%)
					break;
					
					case 'c':											//Copiar el carácter ASCII al formato final
					if (currentParamNumber==1) 
					{
						resultado[message_length++]=(char)val1;
						currentParamNumber++;
					}else if (currentParamNumber==02)
					{
						resultado[message_length++]=(char)val2;
						currentParamNumber++;
					}
					i +=2;												//Se suma 2 porque el formato ocupa 2 ASCII (%c)
					break;
					
					case 'd':
					// Sumar núm de carácteres según tamaño del numero decimal
					if (currentParamNumber<=2)
					{
						int charLeft=10;								// Un int32 ocupara como maximo 10 digitos decimales
						char decimal[charLeft];
						
						if (currentParamNumber == 1)
						{
							_gs_num2str_dec(decimal, charLeft, val1);
							currentParamNumber++;
						} else if (currentParamNumber == 2)
						{
							_gs_num2str_dec(decimal, charLeft, val2);
							currentParamNumber++;
						}
						
						currentChar=decimal;							//puntero para ir iterando sobre la array de chars resultante.
						while (*currentChar==' ') currentChar++;
						while (*currentChar!='\0')
						{
							resultado[message_length++]=*currentChar;
							currentChar++;
						}
					}
					//Si ha superado el límite de parámetros, imprimir tal cual %d
					//Puede que supere el límite de 3 líneas pero se ha guardado 3*VCOLS+1 espacios en la array, por lo cual el último carácter se sustituiría por \0 en escribirlinea
					else
					{
						resultado[message_length++]='%';
						resultado[message_length++]='d';
					}
					i +=2;												//Se suma 2 porque el formato ocupa 2 ASCII (%d)
					break;
					
					case 'x':
					// Sumar núm de carácteres según tamaño del numero hexa
					{
						int charLeft=8;									// Un int32 ocupara como maximo 8 posiciones hexadecimales
						char hexa[charLeft];
						
						if (currentParamNumber == 1)
						{
							_gs_num2str_hex(hexa, charLeft, val1);
							currentParamNumber++;
						} else if (currentParamNumber == 2)
						{
							_gs_num2str_hex(hexa, charLeft, val2);
							currentParamNumber++;
						}
						
						currentChar=hexa;								//puntero para ir iterando sobre la array de chars resultante.
						//controlar que no se introduzca los carácteres del inicio, a no ser que solo queden 2 espacios en el buffer, en ese caso puede que el número sea '0'.
						while ((*currentChar=='0') && (charLeft>2))  currentChar++;
						while (*currentChar!='\0')
						{
							resultado[message_length++]=*currentChar;
							currentChar++;
						}
					}
					i +=2;												//Se suma 2 porque el formato ocupa 2 ASCII (%x)
					break;
					
					case 's':											//Copiar la string al formato final
					{
						char *currentChar='\0';							//Guardar centinela en caso de que supere el numero máximo de parámetros y no se actualice el valor de currentChar
						if (currentParamNumber == 1)
						{
							currentChar=(char *)val1;					//la variable contiene el puntero a la string
							currentParamNumber++;
						}else if (currentParamNumber == 2)
						{
							currentChar=(char *)val2;
							currentParamNumber++;
						}
						
						while ((*currentChar!='\0') && (message_length<3*VCOLS))
						{
							resultado[message_length++]=*currentChar;
							currentChar++;
						}
						i +=2;											//Se suma 2 porque el formato ocupa 2 ASCII (%s)
						break;
					}
					
					case 'l':		// El código es practicamente el mismo para los dos, por eso se hace un fall-through
					case 'L':
					// Sumar núm de carácteres según tamaño del numero long
					// Si no cabe el número long long, se llenaré hasta que se llene el buffer
					{
						int pointed=1;
						if (token[i+1]=='l'){pointed=0;}
						char longlong[26];	//Como máximo un long ocupa 19-20 dígitos. Hay que tener en cuenta el signo y ls puntos '.' -> 26
						
						if (currentParamNumber == 1)
						{
							_gg_long2str(pointed, val1, longlong);
							currentParamNumber++;
						} else if (currentParamNumber == 2)
						{
							_gg_long2str(pointed, val2, longlong);
							currentParamNumber++;
						}
						
						currentChar=longlong;								//puntero para ir iterando sobre la array de chars resultante.
						while (*currentChar!='\0')
						{
							resultado[message_length++]=*currentChar;
							currentChar++;
						}
					}
					i +=2;												//Se suma 2 porque el formato ocupa 2 ASCII (%x)
					break;
					
					default:											//error, no existe el formato, se ignorará.
					{
						i +=2;											//Se ignorará tanto el '%' como la letra de formato inexistente.
						break;
					}
				}
			}
		}else														//Es un char ASCII normal o un carácter de escape
		{
			resultado[message_length++]=formato[i];
			i++;													//Si es un ASCII normal solo se incrementa 1
		}
	}
	resultado[3*VCOLS]='\0';										//Añadir centinela
}


/* _gg_escribir: escribe una cadena de caracteres en la ventana indicada;
	Par metros:
		formato	->	cadena de formato, terminada con centinela '\0';
					admite '\n' (salto de l nea), '\t' (tabulador, 4 espacios)
					y codigos entre 32 y 159 (los 32  ltimos son caracteres
					graficos), adem s de c digos de formato %c, %d, %x y %s
					(max. 2 codigos por cadena)
		val1	->	valor a sustituir en primer c digo de formato, si existe
		val2	->	valor a sustituir en segundo c digo de formato, si existe
					- los valores pueden ser un c digo ASCII (%c), un valor
					  natural de 32 bits (%d, %x) o un puntero a string (%s)
		ventana	->	numero de ventana (de 0 a 3)
		
	Decisió de disseny de _gg_escribir: Si el missatge supera les 3 línies, es ignorarà la resta de contingut que sobrepassa aquest límit.
	Si es sobrepassa per la substitució de format o tabulador / salts de línia, aquest límit es manté.
	Es a dir, si el missatge és "0123\n0123\n3210\nXXXXXXX", es ignorara la part "XXXXXXX" per superar les 3 línies.
*/
void _gg_escribir(char *formato, unsigned int val1, unsigned int val2, int ventana)
{
	// Reservar 3 líneas para la variable que recibe el texto definitivo (+1 para el centinela, ya que no se imprime en pantalla).
	char textoDefinitivo[3*VCOLS+1]="", *token;						//Hay que guardar el centinela
	int filaActual, espaciosAInsertar, charInBuffer, message_length=0;
	
	filaActual=((_gd_wbfs[ventana].pControl >> 16) & 0xffff);		//16 bits altos: número de línea (0-23)
	charInBuffer=(_gd_wbfs[ventana].pControl & 0xFFFF);				//16 bits bajos: caracteres pendientes (0-32)

	_gg_procesarFormato(formato, val1, val2, textoDefinitivo);

	token=textoDefinitivo;
	//Nunca llegará un mensaje más largo que 32 carácteres (sin incluir el centinela). 
	while ((*token!='\0') && (message_length<3*VCOLS))	//Se hace el control de carácteres máximos en procesar formato. Pero \t o \n puede provocar overflow de líneas igualmente (que no se controla en procesarformato).
	{
		if ((charInBuffer==VCOLS) || (*token=='\n'))
		//Comprobar si se ha llenado el buffer o hay salto de linea.
		//Al hacer esta comprovación no hace falta comprovar que charInBuffer<VCOLS (excepto para introducir n espacios).
		{
			swiWaitForVBlank();
			if (filaActual==VFILS)
			{
				_gg_desplazar(ventana);
				filaActual--;										//Escribir en la ultima fila (23)
			}
			_gg_escribirLinea(ventana, filaActual, charInBuffer);
			charInBuffer=0;
			filaActual++;											//Volver a incrementar la fila para la siguiente linea.
			if (*token=='\n')
			{	//Sumar los carácteres restantes de la línea que se han saltado. Ya que aunque no se llenaron sí que ocuparon el resto de la línea.
				message_length=message_length+VCOLS-message_length;
				token++;
			}
		}else if (*token=='\t')
		//Decison de diseño: Si el \t nos causa un salto de linea, se ignoran los espacios que falten
		//Ya que la primera posición de la siguiente fila ya es multiplo de 4
		{
			//Multiplo de 4, si ya lo es hay que insertar 4 espacios para llegar al siguiente multiplo.
			espaciosAInsertar=4-(charInBuffer%4);
			while ((charInBuffer<VCOLS) && (espaciosAInsertar>0) && (message_length<3*VCOLS))
			{
				_gd_wbfs[ventana].pChars[charInBuffer++]=' ';
				message_length++; espaciosAInsertar--;
			}
			token++;
		}else
		//Si no es \t y tampoco es \n  y hay sitio suficiente para un char más en la fila.
		{
			_gd_wbfs[ventana].pChars[charInBuffer++]=*token;
			message_length++;token++;
		} //No hacer nada en caso de no entrar en ningun if, ya que no se imprimió porque la linea esta llena
		
		//Actualizar variable de control
		_gd_wbfs[ventana].pControl=((filaActual & 0xffff) << 16) + (charInBuffer & 0xffff);
	}
}
