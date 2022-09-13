namespace Math
{
	template<typename t>
	t Clamp(t Value, t Min, t Max)
	{
		return Value < Min ? Min : (Value > Max ? Max : Value);
	}
}