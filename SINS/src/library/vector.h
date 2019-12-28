/*
 * vector.h
 *
 *  Created on: Jul 6, 2019
 *      Author: michael
 */

#ifndef VECTOR_H_
#define VECTOR_H_

/*
 * Vector minus vector
 */
void vmv(float a[3], float b[3], float amb[3]);

/*
 * Matrix multiplied by vector
 */
void mxv(float m[3][3], float v[3], float mv[3]);

#endif /* VECTOR_H_ */
