/*
 * Computation of the n'th decimal digit of \pi with very little memory.
 * Written by Fabrice Bellard on January 8, 1997.
 * 
 * We use a slightly modified version of the method described by Simon
 * Plouffe in "On the Computation of the n'th decimal digit of various
 * transcendental numbers" (November 1996). We have modified the algorithm
 * to get a running time of O(n^2) instead of O(n^3log(n)^3).
 * 
 * This program uses mostly integer arithmetic. It may be slow on some
 * hardwares where integer multiplications and divisons must be done
 * by software. We have supposed that 'int' has a size of 32 bits. If
 * your compiler supports 'long long' integers of 64 bits, you may use
 * the integer version of 'mul_mod' (see HAS_LONG_LONG).  
 */
#include "compute_pi_bellards_formula.h"

/* return the inverse of x mod y */
int inv_mod(int x, int y) {
	int q, u, v, a, c, t;

	u = x;
	v = y;
	c = 1;
	a = 0;
	do {
		q = v / u;

		t = c;
		c = a - q * c;
		a = t;

		t = u;
		u = v - q * u;
		v = t;
	} while (u != 0);
	a = a % y;
	if (a < 0)
		a = y + a;
	return a;
}

/* return (a^b) mod m */
int pow_mod(int a, int b, int m) {
	int r, aa;

	r = 1;
	aa = a;
	while (1) {
		if (b & 1)
			r = mul_mod(r, aa, m);
		b = b >> 1;
		if (b == 0)
			break;
		aa = mul_mod(aa, aa, m);
	}
	return r;
}

/* return true if n is prime */
int is_prime(int n) {
	int r, i;
	if ((n % 2) == 0)
		return 0;

	r = (int) (sqrt(n));
	for (i = 3; i <= r; i += 2)
		if ((n % i) == 0)
			return 0;
	return 1;
}

/* return the prime number immediatly after n */
int next_prime(int n) {
	do {
		n++;
	} while (!is_prime(n));
	return n;
}

void compute_n_decimals_of_pi(std::vector<int> &decimals_of_pi,
		int nr_of_decimals) {

	int av, a, vmax, N, n, num, den, k, kq, kq2, t, v, s, i;
	double sum;

	for (n = 1; n < nr_of_decimals; n = n + 9) {
		N = (int) ((n + 20) * log(10) / log(2));

		sum = 0;

		for (a = 3; a <= (2 * N); a = next_prime(a)) {

			vmax = (int) (log(2 * N) / log(a));
			av = 1;
			for (i = 0; i < vmax; i++)
				av = av * a;

			s = 0;
			num = 1;
			den = 1;
			v = 0;
			kq = 1;
			kq2 = 1;

			for (k = 1; k <= N; k++) {

				t = k;
				if (kq >= a) {
					do {
						t = t / a;
						v--;
					} while ((t % a) == 0);
					kq = 0;
				}
				kq++;
				num = mul_mod(num, t, av);

				t = (2 * k - 1);
				if (kq2 >= a) {
					if (kq2 == a) {
						do {
							t = t / a;
							v++;
						} while ((t % a) == 0);
					}
					kq2 -= a;
				}
				den = mul_mod(den, t, av);
				kq2 += 2;

				if (v > 0) {
					t = inv_mod(den, av);
					t = mul_mod(t, num, av);
					t = mul_mod(t, k, av);
					for (i = v; i < vmax; i++)
						t = mul_mod(t, a, av);
					s += t;
					if (s >= av)
						s -= av;
				}

			}

			t = pow_mod(10, n - 1, av);
			s = mul_mod(s, t, av);
			sum = fmod(sum + (double) s / (double) av, 1.0);
		}

		decimals_of_pi.push_back((((int) (sum * 1e9)) / (100000000)) % 10);
		decimals_of_pi.push_back((((int) (sum * 1e9)) / (10000000)) % 10);
		decimals_of_pi.push_back((((int) (sum * 1e9)) / (1000000)) % 10);
		decimals_of_pi.push_back((((int) (sum * 1e9)) / (100000)) % 10);
		decimals_of_pi.push_back((((int) (sum * 1e9)) / (10000)) % 10);
		decimals_of_pi.push_back((((int) (sum * 1e9)) / (1000)) % 10);
		decimals_of_pi.push_back((((int) (sum * 1e9)) / (100)) % 10);
		decimals_of_pi.push_back((((int) (sum * 1e9)) / (10)) % 10);
		decimals_of_pi.push_back(((int) (sum * 1e9)) % (10));

	}
/*	cout << "3 . ";
	for (unsigned i = 0; i < decimals_of_pi.size(); i++) {
		cout << decimals_of_pi[i] << " ";

	}*/
}

int compute_the_nth_decimal(int n) {

	int av, a, vmax, N, num, den, k, kq, kq2, t, v, s, i;
	double sum;

	N = (int) ((n + 20) * log(10) / log(2));

	sum = 0;

	for (a = 3; a <= (2 * N); a = next_prime(a)) {

		vmax = (int) (log(2 * N) / log(a));
		av = 1;
		for (i = 0; i < vmax; i++)
			av = av * a;

		s = 0;
		num = 1;
		den = 1;
		v = 0;
		kq = 1;
		kq2 = 1;

		for (k = 1; k <= N; k++) {

			t = k;
			if (kq >= a) {
				do {
					t = t / a;
					v--;
				} while ((t % a) == 0);
				kq = 0;
			}
			kq++;
			num = mul_mod(num, t, av);

			t = (2 * k - 1);
			if (kq2 >= a) {
				if (kq2 == a) {
					do {
						t = t / a;
						v++;
					} while ((t % a) == 0);
				}
				kq2 -= a;
			}
			den = mul_mod(den, t, av);
			kq2 += 2;

			if (v > 0) {
				t = inv_mod(den, av);
				t = mul_mod(t, num, av);
				t = mul_mod(t, k, av);
				for (i = v; i < vmax; i++)
					t = mul_mod(t, a, av);
				s += t;
				if (s >= av)
					s -= av;
			}
		}

		t = pow_mod(10, n - 1, av);
		s = mul_mod(s, t, av);
		sum = fmod(sum + (double) s / (double) av, 1.0);

	}

	//cout << (int) (sum * 1e9) << endl;
	//cout << (int)sum<<endl;
	return ((((int) (sum * 1e9)) / (100000000)) % 10);
	//decimals_of_pi.push_back((((int)(sum*1e9))/(10000000))%10);
	//decimals_of_pi.push_back((((int)(sum*1e9))/(1000000))%10);
	//decimals_of_pi.push_back((((int)(sum*1e9))/(100000))%10);
	//decimals_of_pi.push_back((((int)(sum*1e9))/(10000))%10);
	//decimals_of_pi.push_back((((int)(sum*1e9))/(1000))%10);
	//decimals_of_pi.push_back((((int)(sum*1e9))/(100))%10);
	//decimals_of_pi.push_back((((int)(sum*1e9))/(10))%10);
	//decimals_of_pi.push_back(((int)(sum*1e9))%(10));


}

