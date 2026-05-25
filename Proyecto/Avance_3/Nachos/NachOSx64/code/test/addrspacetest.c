#include "syscall.h"
int x = SC_Base;
/* Este proceso sirve para probar que el programa cargue correctamente las
	p�ginas en el addrspace.
	Requiere que se encuentre implementado el system call Write() y el system
	call Exit() (aunque �nicamente porque el programa lo llama al final)
	
	Se recomienda que las p�ginas f�sicas en memoria se guarden en desorden (p.e.
	p�gina virtual 1 en p�gina f�sica 2, p�gina virtual 2 en p�gina f�sica 4,
	etc.) 
	
	El programa crear un buffer de 1024 bytes (4 p�ginas) y lo llena con
	27 car�cteres. Si el programa addrspace est� correctamente implementado
	deber�a escribir:
	
abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqr
stuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghi
jklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{
abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqr
stuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghi
jklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{
abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqr
stuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghijklmnopqrstuvwxyz{abcdefghi
jklmnopqrstuvwxy

*/

void main () {
	int i = 0, j = 0;
	char buffer[1024];

	for (j = 0; j<1024;j++) {
		buffer[j]=(char)((j%27)+'a');
	}


	while (i<1) {
		Write(buffer,1024,1);
		i++;
	}
	Exit(0);
} 	
