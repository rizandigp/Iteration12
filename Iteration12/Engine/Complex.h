#pragma once
#include <complex>

// Complex number class
class Complex
{
public:
	float re, im;

	inline Complex()	: re(0.0f), im(0.0f)	{};
	inline Complex( float real, float imaginary )	: re(real), im(imaginary)	{};

	inline Complex conjugate()
	{
		return Complex(this->re,-this->im); 
	}

	inline Complex operator*(const float scalar) const
	{
		return Complex( this->re*scalar, this->im*scalar );
	}
	inline Complex operator*(const Complex &other) const
	{
		return Complex( this->re*other.re - this->im*other.im,
						this->re*other.im + this->im*other.re );
	}
	inline Complex operator/(const float scalar) const
	{
		return Complex( this->re/scalar, this->im/scalar );
	}
	inline Complex operator+(const Complex &other) const
	{
		return Complex( this->re + other.re, this->im + other.im );
	}
	inline Complex operator-(const Complex &other) const
	{
		return Complex( this->re - other.re, this->im - other.im );
	}
	inline Complex& operator=(const Complex &other)
	{
		this->re = other.re;
		this->im = other.im;
		return *this;
	}
	inline Complex& operator=(const float scalar)
	{
		this->re = scalar;
		this->im = 0.0f;
		return *this;
	}
	inline Complex& operator*=(const Complex &other)
	{
		*this = (*this)*other;
		return *this;
	}
	inline Complex& operator*=(const float scalar)
	{
		*this = (*this)*scalar;
		return *this;
	}
	inline Complex& operator/=(const float scalar)
	{
		*this = (*this)/scalar;
		return *this;
	}
};