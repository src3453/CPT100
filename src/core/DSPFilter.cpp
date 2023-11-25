void lowpass(int size, double samplerate, double freq, double q)
{
	double omega = 2.0 * 3.14159265 *  freq　/　samplerate;
	double alpha = sin(omega) / (2.0 * q);
 
	double a0 =  1.0 + alpha;
	double a1 = -2.0 * cos(omega);
	double a2 =  1.0 - alpha;
	double b0 = (1.0 - cos(omega)) / 2.0;
	double b1 =  1.0 - cos(omega);
	double b2 = (1.0 - cos(omega)) / 2.0;
 
	double in1  = 0.0;
	double in2  = 0.0;
	double out1 = 0.0;
	double out2 = 0.0;

}

