/* Post-stack Stolt modeling/migration.
*/
/*
Copyright (C) 2004 University of Texas at Austin

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <math.h>

#include <rsf.h>

#include "cosft.h"

int main(int argc, char* argv[])
{
    int nt,nx,ny, iw,ix,iy, nf, nw;
    float dw, dt, dx,dy, t0, vel, x,y, w,st,sq, *str, *trace2, *trace;
    sf_file in, out;

    sf_init (argc,argv);
    in = sf_input("in");
    out = sf_output("out");

    if (!sf_histint(in,"n1",&nt)) sf_error("No n1= in input");
    if (!sf_histint(in,"n2",&nx)) nx=1;
    if (!sf_histint(in,"n3",&ny)) ny=1;

    if (!sf_getfloat("vel",&vel)) sf_error("Need vel=");
    /* Constant velocity (use negative velocity for modeling) */
    if (!sf_histfloat(in,"d1",&dt)) sf_error("No d1= in input");
    if (!sf_histfloat(in,"o1",&t0)) sf_error("No o1= in input");

    if (!sf_getint ("pad",&nw)) nw=nt;
    /* padding on the time axis */
    nw=2*(nw-1);

    cosft_init(nw);
    dw = 2.*SF_PI/(nw*dt);

    if (!sf_histfloat(in,"d2",&dx)) sf_error("No d2= in input");
    if (!sf_histfloat(in,"d3",&dy)) dy=dx;
    dx *= SF_PI * fabsf (vel);
    dy *= SF_PI * fabsf (vel);	

    if (!sf_getfloat("stretch", &st)) st=1.;
    /* Stolt stretch parameter */
    if (vel < 0) st = 2.-st;

    if (!sf_getint("nf",&nf)) nf=2;
    /* Interpolation accuracy */

    trace = sf_floatalloc(nw);
    trace2 = sf_floatalloc(nw);
    str = sf_floatalloc(nw);

    sf_prefilter_init (nf, nw, 3*nw);
    for (iy = 0; iy < ny; iy++) {
	sf_warning("%d of %d",iy+1,ny);
	y = iy*dy;
	y *= y;
	for (ix = 0; ix < nx; ix++) {
	    x = ix*dx;
	    x = st*(x*x + y);  
	    for (iw = 0; iw < nw; iw++) {
		w = iw*dw;
		sq = (vel < 0)? w*w - x: w*w + x;
		str[iw] = (sq > 0.)? w*(1.-1./st) + sqrtf(sq)/st : - 2.*dw;
	    }
       
	    sf_int1_init (str, 0., dw, nw, sf_spline_int, nf, nw);

	    sf_floatread(trace,nt,in);
	    for (iw = nt; iw < nw; iw++) { /* pad */
		trace[iw]=0.;
	    }
	    cosft_frw (trace,0,1);
	    sf_prefilter_apply (nw, trace);
	    sf_int1_lop (false,false,nw,nw,trace,trace2);
	    cosft_inv (trace2,0,1);
	    sf_floatwrite(trace2,nt,out);
	}
    }

    exit (0);
}

/* 	$Id$	 */
