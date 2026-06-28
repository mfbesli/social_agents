#pragma once

template <typename Int> // Int must be an integral type
Int binomial_coeff(Int n, Int k)
{
	if (k < 0 || n < k)
		return 0;
	else
	{
		Int num = 1;
		Int denom = 1;

		for (Int v = 1; v <= k; v++)
		{
			num *= n - k + v;
			denom *= v;
		}

		return num / denom;
	}
}

