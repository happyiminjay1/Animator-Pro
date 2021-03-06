/***************************************************************
RND file pdr modules:

	Created by Peter Kennard.  Oct 11, 1991
		These modules implement reading scan line and polygon data in
		256 color Autoshade render slide files and drawing the image into
		a screen.  This module by Jim Kent.
****************************************************************/
/* concave.c - a routine to draw a filled polygon with arbatrary
   edge crossings.  Allocates a bit-plane big enough to hold
   rasterized polygon, and then does and xorline/scanedge routine
   that's similar to what drives the Amiga's blitter.  */

#include "errcodes.h"
#include "syslib.h"
#include "rectang.h"
#include "polygon.h"

#define clear_mem(mem,size) memset(mem,0,size) 
#define Bitmap_bpr(w) (((w)+7)>>3) /* bitplane bytes per row for width */

#define UPDIR 1
#define DOWNDIR 0

static Errcode on_off();
static void xor_pt();
static void y_xor_line();

void find_pminmax(Poly *poly, Rectangle *r)
{
register LLpoint *pointpt;
int i;
SHORT xmax,ymax;

pointpt = poly->clipped_list;
r->x = xmax = pointpt->x;
r->y = ymax = pointpt->y;
pointpt = pointpt->next;

i = poly->pt_count;
while (--i > 0)
   {
   if (r->x > pointpt->x) r->x = pointpt->x;
   if (xmax < pointpt->x) xmax = pointpt->x;
   if (r->y > pointpt->y) r->y = pointpt->y;
   if (ymax < pointpt->y) ymax = pointpt->y;
   pointpt = pointpt->next;
   }
r->width = xmax - r->x + 1;
r->height = ymax - r->y + 1;
}

Errcode fill_concave(Poly *poly, EFUNC hline, void *hldat)
{
register LLpoint *pointpt;
register SHORT x,y;
register SHORT ox,oy;
SHORT lastdir;
SHORT i;
SHORT bpr;
long size;
UBYTE *on_off_buf;
Rectangle r;
Errcode err;


find_pminmax(poly, &r);
if (r.height <= 1)  /*can't cope with trivial case*/
	{
	return(Success);
	}
bpr = Bitmap_bpr(r.width);
size = ((long)bpr*r.height);
if ((on_off_buf = malloc(size)) == NULL)	
	return(Err_no_memory);
clear_mem(on_off_buf, size);

pointpt = poly->clipped_list;
x = pointpt->x;
y = pointpt->y;

do
	{
	pointpt = pointpt->next;
	ox = pointpt->x;
	oy = pointpt->y;
	}
while (oy == y);

if (oy>y)
	lastdir = UPDIR;
else
	lastdir = DOWNDIR;

i = poly->pt_count;
while (--i >= 0)
   {
   pointpt = pointpt->next;
   x = pointpt->x;
   y = pointpt->y;
   if (y!=oy)
	  {
	  y_xor_line(on_off_buf, bpr,ox-r.x,oy-r.y,x-r.x,y-r.y);
	  if (y>oy)
		 if (lastdir == UPDIR)
			xor_pt(on_off_buf, bpr, ox-r.x, oy-r.y);
		 else
			lastdir = UPDIR;
	  else
		 if (lastdir == DOWNDIR)
			xor_pt(on_off_buf, bpr, ox-r.x, oy-r.y);
		 else
			lastdir = DOWNDIR;
	  }
   ox = x;
   oy = y;
   }

/*run on off on it*/
err = on_off(bpr, r.width, r.height, r.x, r.y, on_off_buf, hline, hldat);
free(on_off_buf);
return(err);
}



static Errcode on_off(SHORT bpr, SHORT width, SHORT height, 
	SHORT xoff, SHORT yoff, UBYTE *imagept, EFUNC hline, void *hldat)
{
UBYTE *linept;
UBYTE rot;
SHORT start_hseg;
SHORT xpos, xend;
Errcode err = Success;

xend = xoff + width;
while (--height >= 0)
	{
	linept = imagept;
	xpos = xoff;
start_off:
	while ( !*linept)
		{
		linept++;
		if ((xpos += 8) >= xend)
			goto next_line;
		}
	rot = 0x80;
	while (!(rot & *linept))
		{
		if ((xpos += 1) >= xend)
			{
			/* SQUAWK */
			goto next_line;
			}
		rot >>=1;
		} /*scan til first on*/
skip_first_on:
	if ((xpos += 1) >= xend)
		goto next_line;
	start_hseg = xpos;
	rot >>=1;
start_on:
	for (;;)
		{
		if (rot == 0)
			{
			linept++;
			break;
			}
		if ((rot & *linept)) /*if found end segment in word*/
			goto end_in_word;
		if ((xpos += 1) >= xend)
			goto next_line;
		rot >>=1;
		}
	while ( !*linept)
		{
		linept++;
		if ((xpos += 8) >= xend)
			{
			/* SQUAWK */
			goto next_line;
			}
		}
	rot = 0x80;
end_in_word:
	for (;;) 
		{
		if ( (rot & *linept) ) /* Found end of segment. */
			{
			if ((err = (*hline)(yoff, start_hseg, xpos, hldat)) != 0)
				goto DONE;
			for (;;)
				{
				if ((xpos += 1) >= xend)
				  goto next_line;
				rot >>= 1;
				if (rot == 0)
				  {
				  linept++;
				  goto start_off;
				  }
				if (rot & *linept)
				  goto skip_first_on;
				}
			}
		if ((xpos += 1) >= xend)
			goto next_line;
		rot >>= 1;
		}
	next_line:
	imagept += bpr;
	yoff++;
	}
DONE:
	return(err);
}


static void xor_pt(UBYTE *buf, SHORT bpr,register SHORT x,SHORT y)
{
register UBYTE rot;

rot = ((unsigned)0x80) >> (x&7);
buf[ bpr*y + (x>>3) ] ^= rot;
}


static void y_xor_line(UBYTE *imagept, SHORT bpr,int x1,int y1,int x2,int y2)
{
register UBYTE rot;
register SHORT   duty_cycle;
register SHORT   delta_x, delta_y;
register SHORT dots;
int swap;

if (x1 > x2)
	{
	swap = x1;
	x1 = x2;
	x2 = swap;
	swap = y1;
	y1 = y2;
	y2 = swap;
	}
delta_y = y2-y1;
delta_x = x2-x1;
rot = ((unsigned)0x80) >> (x1&7);
imagept += bpr*y1 + (x1>>3);


if (delta_y < 0) 
	{
	delta_y = -delta_y;
	bpr = -bpr;
	}
duty_cycle = (delta_x - delta_y)/2;
*(imagept) ^= rot;
if (delta_x >= delta_y)
	goto horizontal;
dots = delta_y++;
while (--dots >= 0)
	{
	*(imagept) ^= rot;
	duty_cycle += delta_x;	  /* update duty cycle */
	imagept += bpr;
	if (duty_cycle > 0)
		{
		duty_cycle -= delta_y;
		rot >>= 1;
		if (rot == 0)
			{
			imagept++;
			rot = 0x80;
			}
		}
	}
return;

horizontal:
dots = delta_x++;
while (--dots >= 0)
	{
	duty_cycle -= delta_y;	  /* update duty cycle */
	if (duty_cycle < 0)
		{
		*(imagept) ^= rot;
		duty_cycle += delta_x;
		imagept += bpr;
		}
	rot >>= 1;
	if (rot == 0)
		{
		imagept++;
		rot = 0x80;
		}
	}
return;
}



 
