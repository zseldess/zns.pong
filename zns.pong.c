/*
 zns.pong - a Max version of pong~
	
	Developed by Zachary Seldess, King Abdullah University of Science and Technology
 */

#include "ext.h" // standard Max include, always required
#include "ext_obex.h" // required for new style Max object
#include <math.h>

////////////////////////// object struct
typedef struct _pong 
{
    t_object s_obj; // the object itself (must be first)
    void *m_outlet1; // left outlet
    double s_value; // value to pong
    long s_mode; // pong mode
    double s_loVal; // pong low value
    double s_hiVal; // pong high value
} t_pong;

///////////////////////// function prototypes
void *pong_new(t_symbol *s, long argc, t_atom *argv);
void pong_free(t_pong *x);
void pong_assist(t_pong *x, void *b, long m, long a, char *s);

void pong_float(t_pong *x, double f);
void pong_in1(t_pong *x, double n); // middle inlet
void pong_in2(t_pong *x, double n); // right inlet
void pong_anything(t_pong *x, t_symbol *s, long argc, t_atom *argv);
void pong_bang(t_pong *x);

//////////////////////// global class pointer variable
void *pong_class;


int main(void)
{	
    t_class *c;

    c = class_new("zns.pong", (method)pong_new, (method)pong_free, (long)sizeof(t_pong), 0L, A_GIMME, 0);
	 
    class_addmethod(c, (method)pong_float, "float", A_FLOAT, 0);	
    class_addmethod(c, (method)pong_in1, "ft1", A_FLOAT, 0);
    class_addmethod(c, (method)pong_in2, "ft2", A_FLOAT, 0);
    class_addmethod(c, (method)pong_anything, "anything", A_GIMME, 0);
    class_addmethod(c, (method)pong_bang, "bang", 0);
    
    class_addmethod(c, (method)stdinletinfo, "inletinfo", A_CANT, 0); // all right inlets are cold
    class_addmethod(c, (method)pong_assist, "assist", A_CANT, 0); // you CAN'T call this from the patcher

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    pong_class = c;

    return 0;
}

void *pong_new(t_symbol *s, long argc, t_atom *argv)
{
    t_pong *x = NULL;
    long i; 
	
    if ((x = (t_pong *)object_alloc(pong_class))) {
        floatin(x, 2); // right inlet
        floatin(x, 1); // middle inlet
        x->m_outlet1 = floatout((t_object *)x); // left outlet
        // another way to do this
        //x->m_outlet1 = outlet_new((t_object *)x, "float"); // left outlet
        /*
        object_post((t_object *)x, "a new %s object was instantiated: 0x%X", s->s_name, x);
        object_post((t_object *)x, "it has %ld arguments", argc);
        // report all provided arguments
        for (i = 0; i < argc; i++) {
            if ((argv + i)->a_type == A_LONG) {
                object_post((t_object *)x, "arg %ld: long (%ld)", i, atom_getlong(argv+i));
            } 
            else if ((argv + i)->a_type == A_FLOAT) {
                object_post((t_object *)x, "arg %ld: float (%f)", i, atom_getfloat(argv+i));
            } 
            else if ((argv + i)->a_type == A_SYM) {
                object_post((t_object *)x, "arg %ld: symbol (%s)", i, atom_getsym(argv+i)->s_name);
            }
            else {
                object_error((t_object *)x, "forbidden argument");
            }
        }
        */
        // set default object state
        x->s_mode = 0;
        x->s_loVal = 0.0;
        x->s_hiVal = 1.0;
        
        // initialize with args, if provided (3 optional)
        for (i = 0; i < argc; i++) {
            if ((argv + i)->a_type == A_LONG || (argv + i)->a_type == A_FLOAT) {
                switch (i) {
                    case 0:
                        x->s_mode = atom_getfloat(argv + i);
                        break;
                    case 1:
                        x->s_loVal = atom_getfloat(argv + i);
                        break;
                    case 2:
                        x->s_hiVal = atom_getfloat(argv + i);
                    default:
                        break;
                } 
            }
        }
    }
    return (x);
}

void pong_free(t_pong *x)
{
    ;
}

void pong_assist(t_pong *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { // inlet
        switch (a) {
            case 0:
                sprintf(s, "(float) Input");
                break;
            case 1:
                sprintf(s, "(float) Lo val");
                break;
            case 2:
                sprintf(s, "(float) Hi val");
                break;
            default:
                break;
        }
        //sprintf(s, "I am inlet %ld", a);
    } 
    else {	// outlet
        sprintf(s, "(float) Output"); 
        //sprintf(s, "I am outlet %ld", a);
    }
}

void pong_anything(t_pong *x, t_symbol *s, long argc, t_atom *argv)
{
    /*
    // report message
    long i;
    t_atom *ap;
    
    post("message selector is %s",s->s_name);
    post("there are %ld arguments",argc);
    // increment ap each time to get to the next atom
    for (i = 0, ap = argv; i < argc; i++, ap++) {
        switch (atom_gettype(ap)) {
            case A_LONG:
                post("%ld: %ld",i+1,atom_getlong(ap));
                break;
            case A_FLOAT:
                post("%ld: %.2f",i+1,atom_getfloat(ap));
                break;
            case A_SYM:
                post("%ld: %s",i+1, atom_getsym(ap)->s_name);
                break;
            default:
                post("%ld: unknown atom type (%ld)", i+1, atom_gettype(ap));
                break;
        }
    }
    */
    // if "mode" selector, attempt to set pong mode
    t_symbol *sel_mode;
    sel_mode = gensym("mode");
    if (s == sel_mode && argc >= 1) {
        // initialize with 1st arg, if given
        if (argv->a_type == A_LONG || argv->a_type == A_FLOAT) {
            long n = atom_getlong(argv);
            if (n == 0) {
                x->s_mode = 0;
            }
            else {
                x->s_mode = 1;
            }
        } 
    }
}

void pong_in2(t_pong *x, double n)
{
    x->s_hiVal = n;
}

void pong_in1(t_pong *x, double n)
{
    x->s_loVal = n;
}

void pong_float(t_pong *x, double n)
{
    double lo, hi, spread, modNum;

    if (x->s_loVal <= x->s_hiVal) {
        lo = x->s_loVal;
        hi = x->s_hiVal;
    }
    else {
        lo = x->s_hiVal;
        hi = x->s_loVal;
    }
    spread = hi - lo;
    
    if (x->s_mode == 0) { // folding
        double tempDiff;
        if (n < lo) {
            tempDiff = fmod((lo - n), (spread * 2.0));
            if (tempDiff <= spread) {
                modNum = lo + tempDiff;
            }
            else {
                modNum = hi - (tempDiff - spread);
            }
        }
        else if (n > hi) {
            tempDiff = fmod((n - hi), (spread * 2.0));
            if (tempDiff <= spread) {
                modNum = hi - tempDiff;
            }
            else {
                modNum = lo + (tempDiff - spread);
            }
        }
        else {
            modNum = n;
        }
        
    }
    else { // wrapping
        double tempMod;
        if (n < lo) {
            tempMod = fmod(((lo - n) / spread), 1.0);
            modNum = hi - (tempMod * spread);
        }
        else if (n > hi) {
            tempMod = fmod(((n - hi) / spread), 1.0);
            modNum = lo + (tempMod * spread);
        }
        else modNum = n;
    }
    x->s_value = modNum;
    outlet_float(x->m_outlet1, x->s_value);
}

void pong_bang(t_pong *x)
{
    outlet_float(x->m_outlet1, x->s_value);
}
