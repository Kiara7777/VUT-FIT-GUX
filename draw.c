/*
 * xdraw.c - Base for 1. project
 */
 
 /*
  * Elipsa se pri typu Double Dash nekdy na ruznuch pocitatich chova divne
  */

/*
 * Standard XToolkit and OSF/Motif include files.
 */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h> 

/*
 * Public include files for widgets used in this file.
 */
#include <Xm/MainW.h> 
#include <Xm/Form.h> 
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>

/*
 * Common C library include files
 */
#include <stdio.h>
#include <stdlib.h>

/*
 * Shared variables
 */
#define LINES_ALLOC_STEP	10	/* memory allocation stepping */

GC drawGC = 0;			/* GC used for final drawing */
GC inputGC = 0;			/* GC used for drawing current position */

int x1, y1, x2, y2;		/* input points */ 
int button_pressed = 0;		/* input state */

Widget question;

Atom wm_delete;

Display *display;
Colormap cmap;

enum types
{
  POINT = 0,
  LINE,
  RECTANGLE,
  ELLIPSE,
};

enum colorsL
{
  BLACK = 0,
  WHITE,
  BLUE,
  GREEN,
};

enum colorsF
{
  WHITEF = 0,
  BLACKF,
  YELLOW,
  RED,
};

enum lineT
{
  SOLID = 0,
  DOUBLEDASH,
};



typedef struct params
{
  int typeOBJ;
  int full;
  int lineColor;
  int fullColor;
  int lineType;
  int lineWight;
} PARAMS;

PARAMS result =
{
	.typeOBJ = POINT,
	.full = 0,
	.lineColor = BLACK,
	.fullColor = WHITEF,
	.lineType = SOLID,
	.lineWight = 0,
};

typedef struct draw
{
	PARAMS parametry;
	GC gc;
	int x1, x2, y1, y2; //x2 a y2 poslouzi jako druhe souradnice pro obdelnik a elipsu
} DRAW;


DRAW *draws = NULL;		/* array of line descriptors */
int maxlines = 0;		/* space allocated for max lines */
int nlines = 0;

int gcChangedInput = 1;
int gcChangedDraw = 1;

Pixel	fg, bg;

/*------------------------------------------------------------------------------------------------------*/

void setInputGC(Widget w) {
	
	if (!inputGC) {
		inputGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, NULL);
	}
	
	if (gcChangedInput) {
		XSetFunction(XtDisplay(w), inputGC, GXxor);
		XSetPlaneMask(XtDisplay(w), inputGC, ~0);
		XtVaGetValues(w, XmNforeground, &fg, XmNbackground, &bg, NULL);
		

		XSetForeground(XtDisplay(w), inputGC, result.lineColor ^ bg);
			
		//typ a tloustka cary
		if (result.lineType == DOUBLEDASH)
			XSetLineAttributes(XtDisplay(w), inputGC, result.lineWight, LineDoubleDash, CapRound, JoinRound);
		else
			XSetLineAttributes(XtDisplay(w), inputGC, result.lineWight, LineSolid, CapRound, JoinRound);
		
		
		gcChangedInput = 0;
	}
}


void setDrawGC(Widget w) {
	
	Arg al[4];
    int ac;
    XGCValues v;
	
	
	if (!drawGC) {
		ac = 0;
		XtSetArg(al[ac], XmNforeground, &v.foreground); ac++;
		XtGetValues(w, al, ac);	
		drawGC = XCreateGC(XtDisplay(w), XtWindow(w), GCForeground, &v);
	}
	
	if (gcChangedDraw) {
		
		XSetForeground(XtDisplay(w), drawGC, result.lineColor); //barva
		
		//typ a tloustka cary
		if (result.lineType == DOUBLEDASH)
			XSetLineAttributes(XtDisplay(w), drawGC, result.lineWight, LineDoubleDash, CapRound, JoinRound);
		else
			XSetLineAttributes(XtDisplay(w), drawGC, result.lineWight, LineSolid, CapRound, JoinRound);
		
		gcChangedDraw = 0;
	}
		
	
}

/*
 * Set color line
 */
void colourL() {
	int n = 0;
    Arg args[1];
    XColor xcolour, spare;	/* xlib color struct */
	String barva;
	
	switch(result.lineColor) {
		case WHITE:
			barva = "White";
			break;
		case BLACK:
			barva = "Black";
			break;
		case BLUE:
			barva = "Blue";
			break;
		case GREEN:
			barva = "Green";
			break;
		default:
			barva = "Black";
			
	}
	
	
    if (XAllocNamedColor(display, cmap, barva, &xcolour, &spare) == 0)
        return;			
    result.lineColor = xcolour.pixel;
	
}
/*
 * Set color fill
 */
void colourF() {
	
	XColor xcolourF, spareF;
	String barvaF;
	
	switch (result.fullColor) {
		case WHITEF:
			barvaF = "White";
			break;
		case BLACKF:
			barvaF = "Black";
			break;
		case YELLOW:
			barvaF = "Yellow";
			break;
		case RED:
			barvaF = "Red";
			break;
		default:
			barvaF = "White";
	}
		
	if (XAllocNamedColor(display, cmap, barvaF, &xcolourF, &spareF) == 0)
		return;
		
	result.fullColor = xcolourF.pixel;
}

/*
 * Set coordinates for rectangle
 */
void setRectangle(int *x1, int *x2, int *y1, int *y2, int Px1, int Px2, int Py1, int Py2) {
	
	if (Px1 < Px2) {
		*x1 = Px1;
		*x2 = Px2;
	} else {
		*x1 = Px2;
		*x2 = Px1;
	}
	 
					
	if (Py1 < Py2) {
		*y1 = Py1;
		*y2 = Py2;
	} else {
		*y1 = Py2;
		*y2 = Py1;
	}
}

/*
 * Set coordinates for elipsa
 */
void setEllipsa(int *dx, int *dy, int x1, int x2, int y1, int y2) {
	
	*dx = abs(x2-x1);
	*dy = abs(y2-y1);
}

void InputShape(Widget w, XtPointer client_data, XEvent *event, Boolean *cont)
{
  int xP1, xP2, yP1, yP2, dx, dy;

  if (button_pressed) {
	  setInputGC(w);
	//POINT.... netvorim tahnutim ale klikanim.....
  if (button_pressed > 1) {
	    /* erase previous position */
	    //XDrawLine(XtDisplay(w), XtWindow(w), inputGC, x1, y1, x2, y2);
		switch(result.typeOBJ) {
			case LINE:
				XDrawLine(XtDisplay(w), XtWindow(w), inputGC, x1, y1, x2, y2);
				break;
			case RECTANGLE:
				setRectangle(&xP1, &xP2, &yP1, &yP2, x1, x2, y1, y2);
				if (result.full) {
					XSetForeground(XtDisplay(w), inputGC, result.fullColor ^ bg); //zmenit barvu
					XFillRectangle(XtDisplay(w), XtWindow(w), inputGC, xP1, yP1, xP2-xP1, yP2-yP1);
					XSetForeground(XtDisplay(w), inputGC, result.lineColor ^ bg); //vratit barvu pro obrys
				}
				XDrawRectangle(XtDisplay(w), XtWindow(w), inputGC, xP1, yP1, xP2-xP1, yP2-yP1);
					break;
			case ELLIPSE:
				setEllipsa(&dx, &dy, x1, x2, y1, y2);
				if (result.full) {
					XSetForeground(XtDisplay(w), inputGC, result.fullColor ^ bg); //zmenit barvu
					XFillArc(XtDisplay(w), XtWindow(w), inputGC, x1-dx, y1-dy, 2*dx, 2*dy, 0, 360*64);
					XSetForeground(XtDisplay(w), inputGC, result.lineColor ^ bg); //vratit barvu pro obrys
				}
				XDrawArc(XtDisplay(w), XtWindow(w), inputGC, x1-dx, y1-dy, 2*dx, 2*dy, 0, 360*64);
				break;	
		}
	} else {
	    /* remember first MotionNotify */
	    button_pressed = 2;
	}

	x2 = event->xmotion.x;
	y2 = event->xmotion.y;

	switch(result.typeOBJ) {
		case LINE:
			XDrawLine(XtDisplay(w), XtWindow(w), inputGC, x1, y1, x2, y2);
			break;
		case RECTANGLE:
			setRectangle(&xP1, &xP2, &yP1, &yP2, x1, x2, y1, y2);
			if (result.full) {
				XSetForeground(XtDisplay(w), inputGC, result.fullColor ^ bg); //zmenit barvu
				XFillRectangle(XtDisplay(w), XtWindow(w), inputGC, xP1, yP1, xP2-xP1, yP2-yP1);
				XSetForeground(XtDisplay(w), inputGC, result.lineColor ^ bg); //vratit barvu pro obrys
			}
			XDrawRectangle(XtDisplay(w), XtWindow(w), inputGC, xP1, yP1, xP2-xP1, yP2-yP1);
			break;
		case ELLIPSE:
			setEllipsa(&dx, &dy, x1, x2, y1, y2);
			if (result.full) {
				XSetForeground(XtDisplay(w), inputGC, result.fullColor ^ bg); //zmenit barvu
				XFillArc(XtDisplay(w), XtWindow(w), inputGC, x1-dx, y1-dy, 2*dx, 2*dy, 0, 360*64);
				XSetForeground(XtDisplay(w), inputGC, result.lineColor ^ bg); //vratit barvu pro obrys
			}
			XDrawArc(XtDisplay(w), XtWindow(w), inputGC, x1-dx, y1-dy, 2*dx, 2*dy, 0, 360*64);
			break;	
	}
}
}


/*
 * "DrawLine" callback function
 */
void DrawLineCB(Widget w, XtPointer client_data, XtPointer call_data)
{
	int xP1, xP2, yP1, yP2, dx, dy;
    XmDrawingAreaCallbackStruct *d = (XmDrawingAreaCallbackStruct*) call_data;

    switch (d->event->type) {
	case ButtonPress:
	    if (d->event->xbutton.button == Button1) {
		button_pressed = 1;
		x1 = d->event->xbutton.x;
		y1 = d->event->xbutton.y;
	    }
	    break;

	case ButtonRelease:
	    if (d->event->xbutton.button == Button1) {
		if (++nlines > maxlines) {
		    maxlines += LINES_ALLOC_STEP;
			//void *realloc(void *ptr, size_t size)
			  
			draws = (DRAW*) realloc(draws, sizeof(DRAW) * maxlines);
		}
		
		switch (result.typeOBJ) {
			case POINT:
				draws[nlines - 1].parametry.typeOBJ = POINT;
				draws[nlines - 1].parametry.lineWight = result.lineWight;
				draws[nlines - 1].x1 = d->event->xbutton.x;
				draws[nlines - 1].y1 = d->event->xbutton.y;
				break;
			case LINE:
				draws[nlines - 1].parametry.typeOBJ = LINE;
				draws[nlines - 1].x1 = x1;
				draws[nlines - 1].y1 = y1;
				draws[nlines - 1].x2 = d->event->xbutton.x;
				draws[nlines - 1].y2 = d->event->xbutton.y;
				break;
			case RECTANGLE:
				setRectangle(&xP1, &xP2, &yP1, &yP2, x1, d->event->xbutton.x, y1, d->event->xbutton.y);
				
				draws[nlines - 1].parametry.typeOBJ = RECTANGLE;
				draws[nlines - 1].parametry.full = result.full;
				draws[nlines - 1].x1 = xP1;
				draws[nlines - 1].y1 = yP1;
				draws[nlines - 1].x2 = xP2 - xP1;
				draws[nlines - 1].y2 = yP2 - yP1;
				break;
			case ELLIPSE:
				setEllipsa(&dx, &dy, x1, d->event->xbutton.x, y1, d->event->xbutton.y);
				
				draws[nlines - 1].parametry.typeOBJ = ELLIPSE;
				draws[nlines - 1].parametry.full = result.full;
				draws[nlines - 1].x1 = x1 - dx;
				draws[nlines - 1].y1 = y1 - dy;
				draws[nlines - 1].x2 = 2*dx;;
				draws[nlines - 1].y2 = 2*dy;
				break;
		}

		button_pressed = 0;

		setDrawGC(w);
		
		draws[nlines - 1].gc = XCreateGC(XtDisplay(w), XtWindow(w), 0, NULL);
		XCopyGC(XtDisplay(w), drawGC, GCLineWidth | GCLineStyle, draws[nlines - 1].gc);
		draws[nlines - 1].parametry.fullColor = result.fullColor;
		draws[nlines - 1].parametry.lineColor = result.lineColor;
		
		//XDrawLine(XtDisplay(w), XtWindow(w), drawGC, x1, y1, d->event->xbutton.x, d->event->xbutton.y);
		switch(result.typeOBJ) {
			case POINT:
				if (result.lineWight != 0) //tluste
					XFillArc(XtDisplay(w), XtWindow(w), drawGC, d->event->xbutton.x, d->event->xbutton.y, result.lineWight, result.lineWight, 0, 360*64);
				else //normal
					XDrawPoint(XtDisplay(w), XtWindow(w), drawGC, d->event->xbutton.x, d->event->xbutton.y);
				break;
			case LINE:
				XDrawLine(XtDisplay(w), XtWindow(w), drawGC, x1, y1, d->event->xbutton.x, d->event->xbutton.y);
				break;
			case RECTANGLE:
				setRectangle(&xP1, &xP2, &yP1, &yP2, x1, d->event->xbutton.x, y1, d->event->xbutton.y);
				if (result.full) {
					XSetForeground(XtDisplay(w), drawGC, result.fullColor); //zmenit barvu
					XFillRectangle(XtDisplay(w), XtWindow(w), drawGC, xP1, yP1, xP2-xP1, yP2-yP1);
					XSetForeground(XtDisplay(w), drawGC, result.lineColor); //vratit barvu pro obrys
				}
				XDrawRectangle(XtDisplay(w), XtWindow(w), drawGC, xP1, yP1, xP2-xP1, yP2-yP1);
				break;
			case ELLIPSE:
				setEllipsa(&dx, &dy, x1, d->event->xbutton.x, y1, d->event->xbutton.y);
				if (result.full) {
					XSetForeground(XtDisplay(w), drawGC, result.fullColor); //zmenit barvu
					XFillArc(XtDisplay(w), XtWindow(w), drawGC, x1-dx, y1-dy, 2*dx, 2*dy, 0, 360*64);
					XSetForeground(XtDisplay(w), drawGC, result.lineColor); //vratit barvu pro obrys
				}
				XDrawArc(XtDisplay(w), XtWindow(w), drawGC, x1-dx, y1-dy, 2*dx, 2*dy, 0, 360*64);
				break;	
	    }
	    break;
    }}
}

/*
 * "Expose" callback function
 */
/* ARGSUSED */
void ExposeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (nlines <= 0)
		return;

	for (int i = 0; i < nlines; i++) {
		
		XSetForeground(XtDisplay(w), draws[i].gc, draws[i].parametry.lineColor);
		
		switch (draws[i].parametry.typeOBJ) {
			case POINT:
				if (result.lineWight != 0) //tluste
					XFillArc(XtDisplay(w), XtWindow(w), draws[i].gc, draws[i].x1, draws[i].y1, draws[i].parametry.lineWight, draws[i].parametry.lineWight, 0, 360*64);
				else
					XDrawPoint(XtDisplay(w), XtWindow(w), draws[i].gc, draws[i].x1, draws[i].y1);
				break;
			case LINE:
				XDrawLine(XtDisplay(w), XtWindow(w), draws[i].gc, draws[i].x1, draws[i].y1, draws[i].x2, draws[i].y2);
				break;
			case RECTANGLE:
				if (draws[i].parametry.full) {
					XSetForeground(XtDisplay(w), draws[i].gc, draws[i].parametry.fullColor); //zmenit barvu
					XFillRectangle(XtDisplay(w), XtWindow(w), draws[i].gc, draws[i].x1, draws[i].y1, draws[i].x2,draws[i].y2);
					XSetForeground(XtDisplay(w), draws[i].gc, draws[i].parametry.lineColor); //vratit barvu pro obrys
				}
				XDrawRectangle(XtDisplay(w), XtWindow(w), draws[i].gc, draws[i].x1, draws[i].y1, draws[i].x2,draws[i].y2);
				break;
			case ELLIPSE:
				if (draws[i].parametry.full) {
					XSetForeground(XtDisplay(w), draws[i].gc, draws[i].parametry.fullColor); //zmenit barvu
					XFillArc(XtDisplay(w), XtWindow(w), draws[i].gc, draws[i].x1, draws[i].y1, draws[i].x2, draws[i].y2, 0, 360*64);
					XSetForeground(XtDisplay(w), draws[i].gc, draws[i].parametry.lineColor); //vratit barvu pro obrys
				}
				XDrawArc(XtDisplay(w), XtWindow(w), draws[i].gc,  draws[i].x1, draws[i].y1, draws[i].x2, draws[i].y2, 0, 360*64);
				break;	
				
		}
	}

	
}

/*
 * "Clear" button callback function
 */
/* ARGSUSED */
void ClearCB(Widget w, XtPointer client_data, XtPointer call_data)
{ 
    Widget wcd = (Widget) client_data;

    nlines = 0;
    XClearWindow(XtDisplay(wcd), XtWindow(wcd));
}

/*
 * "Quit" button callback function
 */
/* ARGSUSED */
void QuitCB(Widget w, XtPointer client_data, XtPointer call_data)
{ 
	XtManageChild(question);
}

/*
 * cleaning
 */
void questionCB(Widget w, XtPointer client_data, XtPointer call_data)
{
  switch ((int)client_data)
  {
    case 0: /* ok */
	    for (int i = 0; i < maxlines; i++) {
			if (draws && draws[i].gc)
				XFreeGC(XtDisplay(w), draws[i].gc);
		}
		if (drawGC)
			XFreeGC(XtDisplay(w), drawGC);
		
		if (inputGC)
			XFreeGC(XtDisplay(w), inputGC);
		
      exit(0);
      break;
    case 1: /* cancel */
      break;
  }
}

void quitCB(Widget w, XtPointer client_data, XtPointer call_data)
{
	XtManageChild(question);
}


/*
 * Set object type
 */
void TypObjektu(Widget w, XtPointer client_data, XtPointer call_data)
{
	int i = (int) client_data;
	result.typeOBJ = i;  //POINT, LINE, RECTANGLE, ELLIPSE
	
}

/*
 * Set if fill object
 */
void VyplnitObjekt(Widget w, XtPointer client_data, XtPointer call_data)
{
	int i = (int) client_data;
	result.full = i; //NO, YES
}

/*
 * Set line type
 */
void LineChoser(Widget w, XtPointer client_data, XtPointer call_data)
{
	int i = (int) client_data;
	result.lineType = i; //SOLID, DOUBLEDASH
	gcChangedInput = 1;
	gcChangedDraw = 1;
}


/*
 * Set line width
 */
void WidthChoser(Widget w, XtPointer client_data, XtPointer call_data)
{
	int i = (int) client_data;
	if (i == 0)
		result.lineWight = 0;
	else if (i == 1)
		result.lineWight = 3;
	else
		result.lineWight = 8;
	
	gcChangedInput = 1;
	gcChangedDraw = 1;
}

/*
 * Set line color
 */
void ColorLine(Widget w, XtPointer client_data, XtPointer call_data)
{
	int i = (int) client_data;
	result.lineColor = i;
	colourL();
	gcChangedInput = 1;
	gcChangedDraw = 1;
}

/*
 * Set fill color
 */
void ColorFillChoser(Widget w, XtPointer client_data, XtPointer call_data)
{
	int i = (int) client_data;
	result.fullColor = i;
	colourF();
	gcChangedInput = 1;
	gcChangedDraw = 1;
	
}


int main(int argc, char **argv)
{
    XtAppContext app_context;
    Widget topLevel, 
			mainWin, 
				rowColumnMenu,
					optionMenu, vyplnMenu, typCaryMenu, sirkaCaryMenu, barvaCaryMenu, barvaVyplneMenu,
				frame,
					drawArea,
				rowColumnButton, 
					quitBtn, clearBtn;
	
	XmString druh, bod, usecka, obdelnik, elipsa, 
			 vypln, ano, ne,
			 druhCary, plna, carkovana,
			 sirkaCary, s0, s3, s8,
			 barvaCary, bila, cerna, modra, zelena,
			 barvaVyplne, bilaV, cernaV, zlutaV, cervenaV;
			 
	char *fall[] = {
    "*question.dialogTitle: Confirm close",
    "*question.messageString: Are you sure you want to close?",
    "*question.okLabelString: Yes",
    "*question.cancelLabelString: No",
    "*question.messageAlignment: XmALIGNMENT_CENTER",
    NULL
	};

    /*
     * Register the default language procedure
     */
    XtSetLanguageProc(NULL, (XtLanguageProc)NULL, NULL); //staci aby tam vsude bylo null
	
	{
    topLevel = XtVaAppInitialize(
      &app_context,		 	/* Application context */
      "Draw",				/* Application class */
      NULL, 0,				/* command line option list */
      &argc, argv,			/* command line args */
	  fall,
	  NULL,
	  NULL);			/* terminate varargs list */
	  
	question = XmCreateQuestionDialog(topLevel, "question", NULL, 0);
	XtVaSetValues(question, XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL, NULL);
	XtUnmanageChild(XmMessageBoxGetChild(question, XmDIALOG_HELP_BUTTON));
	

    mainWin = XtVaCreateManagedWidget(
      "mainWin",			/* widget name */
      xmMainWindowWidgetClass,		/* widget class */
      topLevel,				/* parent widget*/
      XmNcommandWindowLocation, XmCOMMAND_BELOW_WORKSPACE,
      NULL);	  /* terminate varargs list */
	  
	rowColumnMenu = XtVaCreateManagedWidget(
      "rowColumnMenu",			/* widget name */
      xmRowColumnWidgetClass,		/* widget class */
      mainWin,				/* parent widget */
      XmNentryAlignment, XmALIGNMENT_CENTER,	/* alignment */
      XmNorientation, XmHORIZONTAL,	/* orientation */
      XmNpacking, XmPACK_COLUMN,	/* packing mode */
      NULL);	  /* terminate varargs list */
	  
    rowColumnButton = XtVaCreateManagedWidget(
      "rowColumnButton",			/* widget name */
      xmRowColumnWidgetClass,		/* widget class */
      mainWin,				/* parent widget */
      XmNentryAlignment, XmALIGNMENT_CENTER,	/* alignment */
      XmNorientation, XmHORIZONTAL,	/* orientation */
      XmNpacking, XmPACK_COLUMN,	/* packing mode */
      NULL);
	 
    frame = XtVaCreateManagedWidget(
      "frame",				/* widget name */
      xmFrameWidgetClass,		/* widget class */
      mainWin,				/* parent widget */
      NULL);				/* terminate varargs list */

    drawArea = XtVaCreateManagedWidget(
      "drawingArea",			/* widget name */
      xmDrawingAreaWidgetClass,		/* widget class */
      frame,				/* parent widget*/
      XmNwidth, 200,			/* set startup width */
      XmNheight, 100,			/* set startup height */
      NULL);				/* terminate varargs list */
	  
	  cmap = DefaultColormapOfScreen(XtScreen(drawArea));
	  display = XtDisplay(drawArea);
	
	
    clearBtn = XtVaCreateManagedWidget(
      "Clear",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      rowColumnButton,			/* parent widget*/
      NULL);				/* terminate varargs list */

    quitBtn = XtVaCreateManagedWidget(
      "Quit",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      rowColumnButton,			/* parent widget*/
      NULL);				/* terminate varargs list */
	} 
	
	{/* DRUH OBJEKTU */	
	druh = XmStringCreateLocalized ("Type:");
	bod = XmStringCreateLocalized ("Point");
    usecka = XmStringCreateLocalized ("Line");
    obdelnik = XmStringCreateLocalized ("Rectangle");
	elipsa = XmStringCreateLocalized ("Ellipse");
	
	optionMenu = XmVaCreateSimpleOptionMenu (
		rowColumnMenu, 
		"OptionMenu", 
		druh,
		'T',
		0,
		TypObjektu,
		XmVaPUSHBUTTON, bod, NULL, NULL, NULL,
		XmVaPUSHBUTTON, usecka, NULL, NULL, NULL,
		XmVaPUSHBUTTON, obdelnik, NULL, NULL, NULL,
		XmVaPUSHBUTTON, elipsa, NULL, NULL, NULL,
		NULL);
		
	XmStringFree (bod);
	XmStringFree (usecka);
    XmStringFree (obdelnik);
    XmStringFree (elipsa);
    XmStringFree (druh);
    XtManageChild (optionMenu);
	  /* DRUH OBJEKTU KONEC */}
	
	{/* VYPLNENI OBJEKTU */
	vypln = XmStringCreateLocalized ("Filled:");
	ano = XmStringCreateLocalized ("Yes");
    ne = XmStringCreateLocalized ("No");
	
	vyplnMenu = XmVaCreateSimpleOptionMenu (
		rowColumnMenu, 
		"VyplMenu", 
		vypln,
		'F',
		0,
		VyplnitObjekt,
		XmVaPUSHBUTTON, ne, NULL, NULL, NULL,
		XmVaPUSHBUTTON, ano, NULL, NULL, NULL,
		NULL);
		
	XmStringFree (ano);
	XmStringFree (ne);
    XmStringFree (vypln);
    XtManageChild (vyplnMenu);
	/* VYPLNENI OBJEKTU KONEC*/}
	
	{/* TYP CARY druhCary, plna, carkovana*/
	druhCary = XmStringCreateLocalized ("Line type: ");
	plna = XmStringCreateLocalized ("Full");
    carkovana = XmStringCreateLocalized ("Line Double Dash");
	
	typCaryMenu = XmVaCreateSimpleOptionMenu (
		rowColumnMenu, 
		"TypCaryMenu", 
		druhCary,
		'L',
		0,
		LineChoser,
		XmVaPUSHBUTTON, plna, NULL, NULL, NULL,
		XmVaPUSHBUTTON, carkovana, NULL, NULL, NULL,
		NULL);
		
	XmStringFree (plna);
	XmStringFree (carkovana);
    XmStringFree (druhCary);
    XtManageChild (typCaryMenu);
	/* TYP CARY KONEC*/}
	
	{/* SIRKA CARY*/
	sirkaCary = XmStringCreateLocalized ("Line width: ");
	s0 = XmStringCreateLocalized ("0");
    s3 = XmStringCreateLocalized ("3");
	s8 = XmStringCreateLocalized ("8");
	
	sirkaCaryMenu = XmVaCreateSimpleOptionMenu (
		rowColumnMenu, 
		"SirkaCaryMenu", 
		sirkaCary,
		'W',
		0,
		WidthChoser,
		XmVaPUSHBUTTON, s0, NULL, NULL, NULL,
		XmVaPUSHBUTTON, s3, NULL, NULL, NULL,
		XmVaPUSHBUTTON, s8, NULL, NULL, NULL,
		NULL);
		
	XmStringFree (s0);
	XmStringFree (s3);
	XmStringFree (s8);
    XmStringFree (sirkaCary);
    XtManageChild (sirkaCaryMenu);
	/* SIRKA CARY KONEC*/}
	
	{/* BARVA CARY */
	barvaCary = XmStringCreateLocalized ("Line color: ");
	bila = XmStringCreateLocalized ("White");
	cerna = XmStringCreateLocalized ("Black");
	modra = XmStringCreateLocalized ("Blue");
	zelena = XmStringCreateLocalized ("Green");
	
	barvaCaryMenu = XmVaCreateSimpleOptionMenu (
		rowColumnMenu, 
		"BarvaCaryMenu", 
		barvaCary,
		'P',
		0,
		ColorLine,
		XmVaPUSHBUTTON, cerna, NULL, NULL, NULL,
		XmVaPUSHBUTTON, bila, NULL, NULL, NULL,
		XmVaPUSHBUTTON, modra, NULL, NULL, NULL,
		XmVaPUSHBUTTON, zelena, NULL, NULL, NULL,
		NULL);
		
	XmStringFree (bila);
	XmStringFree (cerna);
	XmStringFree (modra);
    XmStringFree (zelena);
	XmStringFree (barvaCary);
    XtManageChild (barvaCaryMenu);
	/* BARVA CARY KONEC*/}
	
	{/*BARVA VYPLNE*/
	barvaVyplne = XmStringCreateLocalized ("Fill color: ");
	bilaV = XmStringCreateLocalized ("White");
    cernaV = XmStringCreateLocalized ("Black");
	zlutaV = XmStringCreateLocalized ("Yellow");
	cervenaV = XmStringCreateLocalized ("Red");
	
	barvaVyplneMenu = XmVaCreateSimpleOptionMenu (
		rowColumnMenu, 
		"BarvaVyplneMenu", 
		barvaVyplne,
		'B',
		0,
		ColorFillChoser,
		XmVaPUSHBUTTON, bilaV, NULL, NULL, NULL,
		XmVaPUSHBUTTON, cernaV, NULL, NULL, NULL,
		XmVaPUSHBUTTON, zlutaV, NULL, NULL, NULL,
		XmVaPUSHBUTTON, cervenaV, NULL, NULL, NULL,
		NULL);
		
	XmStringFree (bilaV);
	XmStringFree (cernaV);
	XmStringFree (zlutaV);
	XmStringFree (cervenaV);
    XmStringFree (barvaVyplne);
    XtManageChild (barvaVyplneMenu);
	/*BARVA VYPLNE KONES*/}
	
	colourL();
	colourF();

    XmMainWindowSetAreas(mainWin, rowColumnMenu, rowColumnButton, NULL, NULL, frame);

    XtAddCallback(drawArea, XmNinputCallback, DrawLineCB, drawArea);  //udalosti mysi + klavesnice, finalni kresleni
    XtAddEventHandler(drawArea, ButtonMotionMask, False, InputShape, NULL);  //tahnuto mysi
    XtAddCallback(drawArea, XmNexposeCallback, ExposeCB, drawArea); //udalost prekresleni - zmenseni okna, atd...

    XtAddCallback(clearBtn, XmNactivateCallback, ClearCB, drawArea);
    XtAddCallback(quitBtn, XmNactivateCallback, QuitCB, 0);
	
	XtAddCallback(question, XmNokCallback, questionCB, (XtPointer)0);
	XtAddCallback(question, XmNcancelCallback, questionCB, (XtPointer)1);
	
	wm_delete = XInternAtom(XtDisplay(topLevel), "WM_DELETE_WINDOW", False);
	XmAddWMProtocolCallback(topLevel, wm_delete, quitCB, NULL);
	XmActivateWMProtocol(topLevel, wm_delete);
	

    XtRealizeWidget(topLevel); //realizuje objecty, presneji jejich strom

    XtAppMainLoop(app_context); //hlavni cyklus

    return 0;
}

