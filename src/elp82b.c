/*
 * elp82b.c - Routines to implement the ELP 2000-82B lunar theory 
 * Copyright (C) 2010-2011 Shiva Iyer <shiva.iyer AT g m a i l DOT c o m>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * Reference : "Lunar Solution ELP 2000-82B" - M. Chapront-Touze, J. Chapront
 *             <ftp://cdsarc.u-strasbg.fr/pub/cats/VI/79/>
 */

#include <math.h>
#include <memory.h>
#include <kepler.h>
#include <fund_args.h>
#include <elp82b.h>

/* These arrays in elp82b_data.c contain the series of terms in the theory */
extern void *elp_terms[];
extern u_short elp_term_count[];

/*
 * Calculates the Moon's geocentric rectangular coordinates using the
 * ELP 2000-82B lunar theory in its entirety.
 *
 * tdb -- TDB to be used for calculations. TT may be used for all but the most
 *        exacting applications.
 * pos -- The Moon's geocentric rectangular coordinates in KM. The reference
 *        frame is the equinox & ecliptic of J2000.
 */
void elp82b_coordinates(struct julian_date *tdb, struct rectangular_coordinates *pos)
{
	int i,j,k;
	struct elp82b_term1 *p1;
	struct elp82b_term2 *p2;
	struct elp82b_term3 *p3;
	double t,w,T,d,l,lp,f,m,alpha2_3m,d_nu,dnu2_3nu,zeta,me,ve,ma,ju,sa,
		ur,ne,x,y,lbr1[3],lbr2[3],lbr3[3],U,V,r,P,Q,pm[3][3];

	t = JULIAN_CENTURIES(tdb->date1, tdb->date2);

	/* Constants needed for additive corrections to A in the main problem */
	m = 129597742.2758 / 1732559343.73604;
	alpha2_3m = 0.00514376267 / (3.0 * m);
	d_nu = (-0.06424 - m * 0.55604) / 1732559343.73604;
	dnu2_3nu = 1.11208 / 5197678031.20812;

	/*
	 * The following constants, fitted to DE200/LE200, are used to correct A
	 * in the main problem. b2, b3 and b4 in elp82b_data.c have already been
	 * multiplied with these values.
	 * del_e (0".01789), del_eprime (-0".12879), del_g (-0".08066)
	 */
 
	/* 
	 * Delaunay arguments. These expressions are different from the ones used
	 * in the IAU precession model.
	 */
	d  = (1072260.73512 +
		(1602961601.4603 +
		(-5.8681 +
		(0.006595 - 0.00003184 * t) * t) * t) * t) * ACS_TO_RAD;

	lp = (1287104.79306 +
		(129596581.0474 +
		(-0.5529 + 0.000147 * t) * t) * t) * ACS_TO_RAD;

	l  = (485868.28096 +
		(1717915923.4728 +
		(32.3893 +
		(0.051651 - 0.00024470 * t) * t) * t) * t) * ACS_TO_RAD;

	f  = (335779.55755 +
		(1739527263.0983 +
		(-12.2505 +
		(-0.001021 + 0.00000417 * t) * t) * t) * t) * ACS_TO_RAD;

	/*
	 * Longitudes of the planets, used in calculating perturbations. They
	 * are different from the ones used in the IAU precession model.
	 */
	me = (908103.25986 + 538101628.68898 * t) * ACS_TO_RAD;
	ve = (655127.28305 + 210664136.43355 * t) * ACS_TO_RAD;
	ma = (1279559.78866 + 68905077.59284 * t) * ACS_TO_RAD;
	ju = (123665.34212 + 10925660.42861 * t) * ACS_TO_RAD;
	sa = (180278.89694 + 4399609.65932 * t) * ACS_TO_RAD;
	ur = (1130598.01841 + 1542481.19393 * t) * ACS_TO_RAD;
	ne = (1095655.19575 + 786550.32074 * t) * ACS_TO_RAD;

	/*
	 * Mean longitude of the earth. A different expression from the argument
	 * ARG_LONGITUDE_EARTH used in the IAU precession model.
	 */
	T = (361679.22059 +
		(129597742.2758 +
		(-0.0202 +
		(0.000009 + 0.00000015 * t) * t) * t) * t) * ACS_TO_RAD;

	/* Mean longitude of the moon */
	w = fundamental_argument(ARG_LONGITUDE_MOON, t);

	/*
	 * Angle of mean ecliptic of date wrt equinox of J2000, corrected with
	 * the precessional constant.
	 */
	zeta = (785939.95571 + 1732564372.83264 * t) * ACS_TO_RAD;

	memset(lbr1, 0, sizeof(lbr1));
	memset(lbr2, 0, sizeof(lbr2));
	memset(lbr3, 0, sizeof(lbr3));

	/* Main problem, files elp1 to elp3 */
	for (i = 1; i <= 3; i++) {
		k = i - 1;
		p1 = elp_terms[i];
		for (j = elp_term_count[i] - 1; j >= 0; j--) {
			x = p1->i1 * d + p1->i2 * lp + p1->i3 * l + p1->i4 * f;
			y = p1->a + (p1->b1 + p1->b5 * alpha2_3m) * d_nu +
				p1->b2 + p1->b3 + p1->b4;
			if (i != 3) {
				lbr1[k] += y * sin(x);
			} else {
				y -= p1->a * dnu2_3nu;
				lbr1[k] += y * cos(x);
			}
			p1++;
		}
	}

	/* 
	 * Perturbations: Earth (elp4-elp9), tidal effects (elp22-elp27),
	 * Moon (elp28-elp30), relativistic (elp31-elp33), planetary effects
	 * on solar eccentricity (elp34-elp36).
	 */
	for (i = 4; i <= 36; i++) {
		if (i >= 10 && i <= 21)
			continue;

		k = (i - 1) % 3;
		p2 = elp_terms[i];
		for (j = elp_term_count[i] - 1; j >= 0; j--) {
			x = p2->i1 * zeta + p2->i2 * d + p2->i3 * lp +
				p2->i4 * l + p2->i5 * f + p2->phi;
			y = p2->a;
			if ((i >= 7 && i <= 9) || (i >= 25 && i <= 27))
				y *= t;
			if (i >= 34 && i <= 36)
				y = (y * t) * t;
			lbr2[k] += y * sin(x);
			p2++;
		}
	}

	/* Planetary perturbations, files elp10 to elp21 */
	for (i = 10; i <= 21; i++) {
		k = (i - 1) % 3;
		p3 = elp_terms[i];
		for (j = elp_term_count[i] - 1; j >= 0; j--) {
			x = p3->phi + p3->i1 * me + p3->i2 * ve + p3->i3 * T +
				p3->i4 * ma + p3->i5 * ju + p3->i6 * sa +
				p3->i7 * ur + p3->i10 * l + p3->i11 * f;
			if (i <= 15) {
				x += p3->i8 * ne + p3->i9 * d;
			} else {
				x += p3->i8 * d + p3->i9 * lp;
			}
			y = p3->a;
			if ((i >= 13 && i <= 15) || (i >= 19 && i <= 21))
				y *= t;
			lbr3[k] += y * sin(x);
			p3++;
		}
	}

	/* Sum up the individual contributions */
	V = reduce_angle(w + (lbr1[0] + lbr2[0] + lbr3[0]) * ACS_TO_RAD, TWO_PI);
	U = (lbr1[1] + lbr2[1] + lbr3[1]) * ACS_TO_RAD;
	r = (lbr1[2] + lbr2[2] + lbr3[2])* 384747.9806448954 / 384747.9806743165;

	/* Convert to rectangular coordinates */
	pos->x = r * cos(V) * cos(U);
	pos->y = r * sin(V) * cos(U);
	pos->z = r * sin(U);

	/* Set up precession matrix for conversion to the mean ecliptic of J2000 */
	P = (0.10180391E-4 +
		(0.47020439E-6 +
		(-0.5417367E-9 +
		(-0.2507948E-11 +
		0.463486E-14 * t) * t) * t) * t) * t;

	Q = (-0.113469002E-3 +
		(0.12372674E-6 +
		(0.12654170E-8 +
		(-0.1371808E-11 +
		-0.320334E-14 * t) * t) * t) * t) * t;

	pm[0][0] = 1.0 - (2.0 * P) * P;
	pm[0][1] = (2.0 * P) * Q;
	pm[0][2] = (2.0 * P) * sqrt(1.0 - P * P - Q * Q);

	pm[1][0] = pm[0][1];
	pm[1][1] = 1.0 - (2.0 * Q) * Q;
	pm[1][2] = (-2.0 * Q) * sqrt(1.0 - P * P - Q * Q);

	pm[2][0] = -pm[0][2];
	pm[2][1] = -pm[1][2];
	pm[2][2] = 1.0 - (2.0 * P) * P - (2.0 * Q) * Q;

	/* Rotate the moon's coordinates to the mean ecliptic of J2000 */
	rotate_rectangular(pm, pos);
}

/*
 * Rotates the Moon's coordinates from the ecliptic frame of J2000 to the
 * equatorial frame of J2000/FK5.
 *
 * pos -- The coordinates to be rotated in-place
 */
void elp82b_ecliptic_to_equator(struct rectangular_coordinates *pos)
{
	/* Rotation matrix specified in the theory */
	static double rot_matrix[3][3] = {
		{ 1.000000000000, 0.000000437913, -0.000000189859},
		{-0.000000477299, 0.917482137607, -0.397776981701},
		{ 0.000000000000, 0.397776981701,  0.917482137607}
	};

	rotate_rectangular(rot_matrix, pos);
}
