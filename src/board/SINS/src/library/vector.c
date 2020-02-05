/*
 * vector.c
 *
 *  Created on: Jul 6, 2019
 *      Author: michael
 *  From SOFA library
 */

void vmv(float a[3], float b[3], float amb[3])
{
	amb[0] = a[0] - b[0];
	amb[1] = a[1] - b[1];
	amb[2] = a[2] - b[2];

	return;
}


void mxv(float m[3][3], float v[3], float mv[3])
{
	float w, wrp[3];
	int i, j;


	/* Matrix r * vector p. */
	for (j = 0; j < 3; j++) {
	   w = 0.0;
	   for (i = 0; i < 3; i++) {
		   w += m[j][i] * v[i];
	   }
	   wrp[j] = w;
	}

	/* Return the result. */
	mv[0] = wrp[0];
	mv[1] = wrp[1];
	mv[2] = wrp[2];

	return;
}

