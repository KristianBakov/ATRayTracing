#pragma once
#include <math.h>
#include <iostream>
#include <vector>
#include "vec4.h"

class Matrix4
{
protected:
	std::vector<std::vector<float>> Matrix4Data;
public:

	//constructor
	Matrix4() {
		for (size_t i = 0; i < 4; i++)
		{
			std::vector<float> vec;
			for (size_t j = 0; j < 4; j++)
			{
				vec.push_back(0);
			}
			Matrix4Data.push_back(vec);
		}
	}
	template <typename... Args>
	Matrix4(Args... args) {
		float data[4 * 4] = { args... };

		for (size_t i = 0; i < 4; i++)
		{
			std::vector<float> vec;
			for (size_t j = 0; j < 4; j++)
			{
				vec.push_back(data[i * 4 + j]);
			}
			Matrix4Data.push_back(vec);
		}
		static_assert(sizeof...(Args) == 4 * 4, "wrong number of arguments");
	}

	//cop4 constructor
	Matrix4& operator=(const Matrix4& value)
	{
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				Matrix4Data.at(i).at(j) = value.Matrix4Data.at(i).at(j);
			}
		}
		return *this;
	}

	void SetIdentityMatrix()
	{
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				Matrix4Data.at(i).at(j) = (i == j ? 1.0f : 0.0f);
			}
		}
	}

	Matrix4 GetCofactor(const Matrix4& value, int p, int q, int n)
	{
		Matrix4 temp;
		int i = 0, j = 0;

		// Looping for each element of the Matrix4 
		for (int row = 0; row < n; row++)
		{
			for (int col = 0; col < n; col++)
			{
				//  Cop4ing into temporar4 Matrix4 onl4 those element 
				//  which are not in given row and column 
				if (row != p && col != q)
				{
					temp.Matrix4Data.at(i).at(j++) = value.Matrix4Data.at(row).at(col);

					// Row is filled, so increase row inde4 and 
					// reset col inde4 
					if (j == n - 1)
					{
						j = 0;
						i++;
					}
				}
			}
		}
		return temp;
	}

	/* Recursive function for finding determinant of Matrix4.
	   n is current dimension of A[][]. */
	float Determinant(const Matrix4& value, int n)
	{
		float det = 0; // Initialize result 

		//  Base case : if Matrix4 contains single element 
		if (n == 2)
			return (value.Matrix4Data.at(0).at(0) * value.Matrix4Data.at(1).at(1)) - (value.Matrix4Data.at(1).at(0) * value.Matrix4Data.at(0).at(1));

		Matrix4 temp; // To store cofactors 

		int sign = 1;  // To store sign multiplier 

		 // Iterate for each element of first row 
		for (int f = 0; f < n; f++)
		{
			// Getting Cofactor of A[0][f]
			temp = GetCofactor(value, 0, f, n);
			det += sign * value.Matrix4Data.at(0).at(f) * Determinant(temp, n - 1);

			// terms are to be added with alternate sign 
			sign = -sign;
		}

		return det;
	}

	// Function to get adjoint of A[N][N] in adj[N][N]. 
	Matrix4 Adjoint(const Matrix4& value)
	{
		Matrix4 adj, temp;
		if (value.Matrix4Data.size() == 1)
		{
			adj.Matrix4Data.at(0).at(0) = 1;
			return adj;
		}

		// temp is used to store cofactors of A[][] 
		int sign = 1;

		for (int i = 0; i < value.Matrix4Data.size(); i++)
		{
			for (int j = 0; j < value.Matrix4Data.size(); j++)
			{
				// Get cofactor of A[i][j] 
				temp = GetCofactor(value, i, j, value.Matrix4Data.size());

				// sign of adj[j][i] positive if sum of row 
				// and column inde4es is even. 
				sign = ((i + j) % 2 == 0) ? 1 : -1;

				// Interchanging rows and columns to get the 
				// transpose of the cofactor Matrix4 
				adj.Matrix4Data.at(j).at(i) = (sign) * (Determinant(temp, value.Matrix4Data.size() - 1));
			}
		}
		return adj;
	}

	// Function to calculate and store inverse, returns false if 
	// Matrix4 is singular 
	bool SetAsInverseOf(const Matrix4& value)
	{
		// Find determinant of A[][] 
		float det = Determinant(value, value.Matrix4Data.size());
		if (det == 0.0f)
		{
			std::cout << "Singular Matrix4, can't find its inverse" << std::endl;
			return false;
		}

		// Find adjoint 
		Matrix4 adj = Adjoint(value);

		// Find Inverse using formula "inverse(A) = adj(A)/det(A)" 
		for (int i = 0; i < value.Matrix4Data.size(); i++)
			for (int j = 0; j < value.Matrix4Data.size(); j++)
				Matrix4Data.at(i).at(j) = adj.Matrix4Data.at(i).at(j) / det;

		return true;
	}

	Matrix4 GetInverseOf(const Matrix4& value)
	{
		Matrix4 inverse;
		// Find determinant of A[][] 
		float det = Determinant(value, value.Matrix4Data.size());
		if (det == 0.0f)
		{
			std::cout << "Singular Matrix4, can't find its inverse";
			return value;
		}

		// Find adjoint 
		Matrix4 adj = Adjoint(value);

		// Find Inverse using formula "inverse(A) = adj(A)/det(A)" 
		for (int i = 0; i < value.Matrix4Data.size(); i++)
			for (int j = 0; j < value.Matrix4Data.size(); j++)
				inverse.Matrix4Data.at(i).at(j) = adj.Matrix4Data.at(i).at(j) / float(det);

		return inverse;
	}

	void SetAsInverse()
	{
		SetAsInverseOf(*this);
	}

	Matrix4 GetInverse()
	{
		return GetInverseOf(*this);
	}

	Matrix4 GetTranspose()
	{
		return Transpose(*this);
	}
	void Transpose()
	{
		static_assert(4 == 4, "can not set a non squair Matrix4 to a transpose of it's self");
		*this = Transpose(*this);
	}
	Matrix4 Transpose(const Matrix4& value)
	{
		Matrix4 temp;
		for (size_t i = 0; i < value.Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < value.Matrix4Data.front().size(); j++)
			{
				temp.Matrix4Data.at(j).at(i) = value.Matrix4Data.at(i).at(j);
			}
		}
		return temp;
	}

	//operations
	void operator+=(const Matrix4& value)
	{
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				Matrix4Data.at(i).at(j) += value.Matrix4Data.at(i).at(j);
			}
		}
	}
	Matrix4 operator+(const Matrix4& value) const
	{
		Matrix4 mat;
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				mat.Matrix4Data.at(i).at(j) = Matrix4Data.at(i).at(j) + value.Matrix4Data.at(i).at(j);
			}
		}
		return mat;
	}

	void operator-=(const Matrix4& value)
	{
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				Matrix4Data.at(i).at(j) -= value.Matrix4Data.at(i).at(j);
			}
		}
	}
	Matrix4 operator-(const Matrix4& value)
	{
		Matrix4 mat;
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				mat.Matrix4Data.at(i).at(j) = Matrix4Data.at(i).at(j) - value.Matrix4Data.at(i).at(j);
			}
		}
		return mat;
	}

	void operator*=(float s)
	{
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				Matrix4Data.at(i).at(j) *= s;
			}
		}
	}
	Matrix4 operator*(float s) const
	{
		Matrix4 mat;
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				mat.Matrix4Data.at(i).at(j) = Matrix4Data.at(i).at(j) * s;
			}
		}
		return mat;
	}

	void operator/=(const Matrix4& value)
	{
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				Matrix4Data.at(i).at(j) /= value.Matrix4Data.at(i).at(j);
			}
		}
	}
	Matrix4 operator/(const Matrix4& value) const
	{
		Matrix4 mat;
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				mat.Matrix4Data.at(i).at(j) = Matrix4Data.at(i).at(j) / value.Matrix4Data.at(i).at(j);
			}
		}
		return mat;
	}
	void operator/=(float s)
	{
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				Matrix4Data.at(i).at(j) /= s;
			}
		}
	}
	Matrix4 operator/(float s) const
	{
		Matrix4 mat;
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				mat.Matrix4Data.at(i).at(j) = Matrix4Data.at(i).at(j) / s;
			}
		}
		return mat;
	}

	void operator*=(const Matrix4& value)
	{
		Matrix4 mat;
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < value.Matrix4Data.front().size(); j++)
			{
				for (size_t k = 0; k < Matrix4Data.front().size(); k++)
				{
					mat.Matrix4Data.at(i).at(j) += Matrix4Data.at(i).at(k) * value.Matrix4Data.at(k).at(j);
				}
			}
		}
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			std::vector<float> vec;
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				Matrix4Data.at(i).at(j) = mat.Matrix4Data.at(i).at(j);
			}
		}
	}

	Matrix4 operator*(const Matrix4& value) const
	{
		Matrix4 mat;
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < value.Matrix4Data.front().size(); j++)
			{
				for (size_t k = 0; k < Matrix4Data.front().size(); k++)
				{
					mat.Matrix4Data.at(i).at(j) += Matrix4Data.at(i).at(k) * value.Matrix4Data.at(k).at(j);
				}
			}
		}
		return mat;
	}


	void show() {
		std::cout << "[";
		for (size_t i = 0; i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				std::cout << Matrix4Data.at(i).at(j) << ",";
			}
			std::cout << std::endl;
		}
		std::cout << "]" << std::endl;
	}


	vec3 TransformVector(const vec3& vec) const
	{
		vec3 out_vec;
		vec4 temp_vec;

		for (size_t i = 0; i < 4; i++)
		{
			temp_vec.e[i] = i < 3 ? vec.e[i] : 1;
		}

		for (size_t i = 0; i < 3 && i < Matrix4Data.size(); i++)
		{
			for (size_t j = 0; j < Matrix4Data.front().size(); j++)
			{
				out_vec.e[i] += Matrix4Data.at(i).at(j) * temp_vec.e[j];
			}
		}
		return out_vec;
	}

	void TranslateByVector(const vec3& vec)
	{
		Matrix4 temp_Matrix4;
		temp_Matrix4.SetIdentityMatrix();


		for (size_t i = 0; i < 3 && i < Matrix4Data.size(); i++)
		{
			temp_Matrix4.Matrix4Data.at(i).back() = vec.e[i];
		}
		*this *= temp_Matrix4;
	}

	void ScaleByVector(const vec3& vec)
	{
		Matrix4 temp_Matrix4;
		temp_Matrix4.SetIdentityMatrix();
		size_t j = 0;
		for (size_t i = 0; i < 3 && i < Matrix4Data.size() && j < 3 && j < Matrix4Data.size(); i++)
		{
			temp_Matrix4.Matrix4Data.at(i).at(j) = vec.e[i];
			j++;
		}
		*this *= temp_Matrix4;
	}

	///rotate along 4 a4is, pass paramiter as degreas of rotaion
	void PitchMatrix4(const float pitch)
	{
		float _sin = sin(pitch);
		float _cos = cos(pitch);

		Matrix4 temp_Matrix4;
		temp_Matrix4.SetIdentityMatrix();

		temp_Matrix4.Matrix4Data.at(1).at(1) = _cos;
		temp_Matrix4.Matrix4Data.at(2).at(2) = _cos;
		temp_Matrix4.Matrix4Data.at(2).at(1) = _sin;
		temp_Matrix4.Matrix4Data.at(1).at(2) = -_sin;

		*this *= temp_Matrix4;

		static_assert(4 > 3 || 4 > 3, "can not rotate around 4 axis if matric is smaller than 343");
	}

	void YawMatrix(const  float yaw)
	{
		float _sin = sin(yaw);
		float _cos = cos(yaw);

		Matrix4 temp_Matrix4;
		temp_Matrix4.SetIdentityMatrix();

		temp_Matrix4.Matrix4Data.at(0).at(0) = _cos;
		temp_Matrix4.Matrix4Data.at(2).at(2) = _cos;
		temp_Matrix4.Matrix4Data.at(0).at(2) = _sin;
		temp_Matrix4.Matrix4Data.at(2).at(0) = -_sin;

		*this *= temp_Matrix4;

		static_assert(4 > 3 || 4 > 3, "can not rotate around 4 axis if matrix is smaller than 343");
	}

	///rotate along z a4is, pass paramiter as degreas of rotaion
	void RollMatrix4(const float roll)
	{
		float _sin = sin(roll);
		float _cos = cos(roll);

		Matrix4 temp_Matrix4;
		temp_Matrix4.SetIdentityMatrix();

		temp_Matrix4.Matrix4Data.at(0).at(0) = _cos;
		temp_Matrix4.Matrix4Data.at(0).at(0) = _cos;
		temp_Matrix4.Matrix4Data.at(1).at(0) = _sin;
		temp_Matrix4.Matrix4Data.at(0).at(1) = -_sin;

		*this *= temp_Matrix4;

		static_assert(4 > 2 || 4 > 2, "can not rotate around z a4is if matric is smaller than 242");
	}

};