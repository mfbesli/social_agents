#pragma once

template <typename Val>
Val binomial_coeff(Val n, Val k)
{
	if (k < 0 || n < k)
		return 0;
	else
	{
		Val num = 1;
		Val denom = 1;

		for (Val v = 1; v <= k; v++)
		{
			num *= n - k + v;
			denom *= v;
		}

		return num / denom;
	}
}

