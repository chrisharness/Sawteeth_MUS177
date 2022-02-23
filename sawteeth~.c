#include "m_pd.h"
#include <stdlib.h>
#include <math.h>
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ sawteeth~ ----------------------------- */

#define WAVETABLESIZE 1024

static t_class *sawteeth_class;

typedef struct _sawteeth
{
    t_object x_obj; 	/* obligatory header */
    t_float x_f;    	/* place to hold inlet's value if it's set by message */
	t_float *wavetable;	/* a place to hold the squared values */
	t_float phase;		// a float for our phase..
	t_float samplerate;	// a float for our samplerate
	t_float filt_out;	// a float which we will use to do some arithmetic inside our filter
	t_float filt_in;	// a float which we will use to pass into the filter...
	t_float cutoff;		// a cutoff value stored from inlet 1 (our slider) 
	t_float bit_in;		// a float which we will use to pass into the bitcrusher (aka return from filter)
	t_float bit_out;	// a float which we will use to return from the bitcrusher (and eventually return as the output)
	t_float bits;		// a bit value stored from inlet 2 (our number of bits->bitcrusher)
	t_float harmonics;	// number of harmonics to generate for the sawtooth oscillator...

} t_sawteeth;



// filter_perform function, takes 1 parameter, t_sawteeth *x, which is a pointer to our struct...

static t_float filter_perform(t_sawteeth *x){

	float pi;
	pi = 4.0f * atanf(1.0f);

	double tf = tan(pi * (x->cutoff/x->samplerate)); // tangent frequency   
    double c = (tf - 1.0)/(tf + 1.0); // coefficient     
    
    float sample = x->filt_in;   
    x->filt_out = (c*sample) + x->filt_in - (c * x->filt_out); 
    x->filt_in = sample; // remember input     
    return(x->filt_out *0.5); // output   

}

// bitcrush_perform function, takes 1 parameter, t_sawteeth *x, which is a pointer to our struct...


static t_float bitcrush_perform(t_sawteeth *x){

	// decimation to 3 bits using multiplication and division
	float factor,output,bits;
	int input16,output16;

	bits = (float)x->bits;
	// get bits from the inlet...
	// convert to an integer to be safe for our switch statement...
	int int_bits;
	int_bits = (int)bits;


	factor = pow(2.0, bits - 1.0);
	output = floor(x->bit_in * factor)/factor;

	// decimation to 3 bits using logical operations

		// convert to 16-bit sample
	input16 = (int)(x->bit_in * 32768);


	// binary literals only allowed in gcc, in hex use 0xe000
	// switch case -- we will use this in complement with our inlet to determine how many to crush...
	switch(int_bits){

		case 0:
			return(x->bit_in);
			break;

		case 1:

				// zero out all but the top bit using bitwise and
			output16 = input16 & 0b1000000000000000; 
				// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;

			break;
		
		case 2:
				// zero out all but the top 2 bits using bitwise and
			output16 = input16 & 0b1100000000000000; 
				// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;
			break;

		case 3:

			// zero out all but the top 3 bits using bitwise and
			output16 = input16 & 0b1110000000000000; 
			// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;
			break;

		case 4:
		// zero out all but the top 4 bits using bitwise and
			output16 = input16 & 0b1111000000000000; 
				// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;
			break;

		case 5: 
			// zero out all but the top 5 bits using bitwise and
			output16 = input16 & 0b1111100000000000; 
				// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;
			break;
		case 6:
		// zero out all but the top 6 bits using bitwise and
			output16 = input16 & 0b1111110000000000; 
				// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;
			break;
		case 7:
			// zero out all but the top 7 bits using bitwise and
			output16 = input16 & 0b1111111000000000; 
				// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;
			break;
		case 8:
			// zero out all but the top 8 bits using bitwise and
			output16 = input16 & 0b1111111100000000; 
				// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;
			break;
		case 9:
			// zero out all but the top 9 bits using bitwise and
			output16 = input16 & 0b1111111110000000; 
				// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;
			break;
		case 10:
			// zero out all but the top 5 bits using bitwise and
			output16 = input16 & 0b1111111111000000; 
				// convert back to float using 1/32768
			output = output16 * 0.000030517578125;
			x->bit_out = output;
			break;

		default:
			return(x->bit_in);
			break;

	}


	return(x->bit_out);

}



    /* this is the actual performance routine which acts on the samples.
    It's called with a single pointer "w" which is our location in the
    DSP call list.  We return a new "w" which will point to the next item
    after us.  Meanwhile, w[0] is just a pointer to dsp-perform itself
    (no use to us), w[1] and w[2] are the input and output vector locations,
    and w[3] is the number of points to calculate. */




	/// sawteeth perform function

	/// for each sample - performs a low pass @ cutoff frequency + bitcrush (if bits != 0)


static t_int *sawteeth_perform(t_int *w)
{
	t_sawteeth *x = (t_sawteeth *)(w[1]);
    t_float *freq = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);

	int blocksize = n;
	int i, sample = 0;
	float phaseincrement;
	float findex;
	float filterout;
	int	iindex;
	
    while (n--)
    {
		// first we need to calculate the phase increment from the frequency
		// and sample rate - this is the number of cycles per sample
		// freq = cyc/sec, sr = samp/sec, phaseinc = cyc/samp = freq/sr
		phaseincrement = *(freq+sample)/x->samplerate;
		
		// now, increment the phase and make sure it doesn't go over 1.0
		x->phase += phaseincrement;
		while(x->phase >= 1.0f)
			x->phase -= 1.0f;
		while(x->phase < 0.0f)
			x->phase += 1.0f;
		// now grab the sample from the table
		findex = WAVETABLESIZE * x->phase;
		iindex = (int)findex;
		x->filt_in = *(x->wavetable + iindex); // get the sample to put first into the filter...
		x->bit_in = filter_perform(x); // perform the filter and store it in a local variable filterout
		*(out+sample) = bitcrush_perform(x);

		sample++;
    }

    return (w+5);
}

    /* called to start DSP.  Here we call Pd back to add our perform
    routine to a linear callback list which Pd in turn calls to grind
    out the samples. */
static void sawteeth_dsp(t_sawteeth *x, t_signal **sp)
{
	// we'll initialize samplerate when starting up
	x->samplerate = sp[0]->s_sr;
    dsp_add(sawteeth_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *sawteeth_new(void)
{
	float twopi, size;
	int i,k;
    t_sawteeth *x = (t_sawteeth *)pd_new(sawteeth_class);

	// create two inlets, one for cutoff and one for our bits 
	floatinlet_new(&x->x_obj,&x->cutoff);
	floatinlet_new(&x->x_obj,&x->bits);
	
	
    outlet_new(&x->x_obj, gensym("signal"));



	// initialize variables
    x->x_f = 0.0f;
	x->phase = 0.0f;
	twopi = 8.0f * atanf(1.0f);
	size = (float)WAVETABLESIZE;
	
	// space for WAVETABLESIZE samples
	x->wavetable = (t_float *)malloc(WAVETABLESIZE * sizeof(t_float));
	

	int harmonics = 50;// 50 harmonics!! woo
	float amp = 1.0; // amp
	float max = 0; // max for normalizing

	// fill it up 
	for(k = 1; k<=harmonics; k++){


		// based on synth-notes implementation


		for(i = 0; i < WAVETABLESIZE; i++){
			float samp;
			samp = (i*k)/size;
			samp = samp*twopi;
        	*(x->wavetable+i) = *(x->wavetable+i) + (sinf(samp) * 1/harmonics);
			if (*(x->wavetable+i) > max){
				max = *(x->wavetable+i);
			}
		}
	}


	// normalization...

	for(i = 0; i < WAVETABLESIZE; i++){
		*(x->wavetable+i) = *(x->wavetable+i)/max;
	}


	// return normally.
    return (x);
}

// since we allocated some memory, we need a delete function
static void sawteeth_free(t_sawteeth *x)
{
	free(x->wavetable);
}

    /* this routine, which must have exactly this name (with the "~" replaced
    by "_tilde) is called when the code is first loaded, and tells Pd how
    to build the "class". */
void sawteeth_tilde_setup(void)
{
    sawteeth_class = class_new(gensym("sawteeth~"), (t_newmethod)sawteeth_new, (t_method)sawteeth_free,
    	sizeof(t_sawteeth), 0, A_DEFFLOAT, 0);
	    /* this is magic to declare that the leftmost, "main" inlet
	    takes signals; other signal inlets are done differently... */
    CLASS_MAINSIGNALIN(sawteeth_class, t_sawteeth, x_f);
    	/* here we tell Pd about the "dsp" method, which is called back
	when DSP is turned on. */
    class_addmethod(sawteeth_class, (t_method)sawteeth_dsp, gensym("dsp"), 0);
}
