#pragma once


// Complex number class
class Complex
{
public:
	float re, im;

	inline Complex()	: re(0.0f), im(0.0f)	{};
	inline Complex( float Real, float Imaginary )	: re(Real), im(Imaginary)	{};

	inline Complex Conjugate()
	{
		return Complex(this->re,-this->im); 
	}

	inline Complex operator*(const float Scalar) const
	{
		return Complex( this->re*Scalar, this->im*Scalar );
	}
	inline Complex operator*(const Complex &Other) const
	{
		return Complex( this->re*Other.re - this->im*Other.im,
						this->re*Other.im + this->im*Other.re );
	}
	inline Complex operator/(const float Scalar) const
	{
		return Complex( this->re/Scalar, this->im/Scalar );
	}
	inline Complex operator+(const Complex &Other) const
	{
		return Complex( this->re + Other.re, this->im + Other.im );
	}
	inline Complex operator-(const Complex &Other) const
	{
		return Complex( this->re - Other.re, this->im - Other.im );
	}
	inline Complex& operator=(const Complex &Other)
	{
		this->re = Other.re;
		this->im = Other.im;
		return *this;
	}
	inline Complex& operator=(const float Scalar)
	{
		this->re = Scalar;
		this->im = 0.0f;
		return *this;
	}
	inline Complex& operator*=(const Complex &Other)
	{
		*this = (*this)*Other;
		return *this;
	}
	inline Complex& operator*=(const float Scalar)
	{
		*this = (*this)*Scalar;
		return *this;
	}
	inline Complex& operator/=(const float Scalar)
	{
		*this = (*this)/Scalar;
		return *this;
	}
};