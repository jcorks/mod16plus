#ifndef H_SES__LINEAR_ALGEBRA__INCLUDED
#define H_SES__LINEAR_ALGEBRA__INCLUDED





typedef struct {
    float x;
    float y;
    float z;
} sesVector_t;


/// A generic, row-major matrix 
///
typedef struct sesMatrix_t sesMatrix_t;
struct sesMatrix_t {
    /// Data for the matrix, elements are laid out 
    /// linearly in memory, starting with the topleft,
    /// then laying out rows.
    float data[16];
};

/// Sets the matrix to the identity matrix, removing all transformations.
///
void ses_matrix_set_identity(
    /// Matrix to modify.
    sesMatrix_t * m
);

/// Convenience function that transforms the 
/// given point and returns its result.
///
sesVector_t ses_matrix_transform(
    /// The matrix to use as source.
    const sesMatrix_t * m, 

    /// The point to transform.
    const sesVector_t * point
);




/// Transposes the matrix.
///
void ses_matrix_transpose(
    /// The matrix to modify.
    sesMatrix_t * m
);

/// Inverts the matrix.
///
void ses_matrix_invert(
    /// The matrix to modify.
    sesMatrix_t * m
);

/// Reverse the majority of the matrix.
///
void ses_matrix_reverse_majority(
    /// The matrix to modify.
    sesMatrix_t * m
);

/// Returns the internal representation of the TransformMatrix.
///
#define ses_matrix_ptr(__M__) ((float*)__M__.data)


/// Multiplies 2 matrices.
///
sesMatrix_t ses_matrix_multiply(
    /// The first operand of the multiplication.
    const sesMatrix_t * a, 

    /// The second operand of the multiplication.
    const sesMatrix_t * b
);


/// Rotates the matrix about the Euler angles psi, theta, and phi.
///
void ses_matrix_rotate_by_angles(
    /// The transform matrix to modify.
    sesMatrix_t * m,  

    /// The psi (x) rotation angle.
    float x, 

    /// The theta (y) rotation angle.
    float y, 

    /// The phi (z) rotation angle.
    float z
);

/// Expresses a translation by x, y, and z
///
void ses_matrix_translate(
    /// The transform matrix to modify.
    sesMatrix_t * m,  

    /// The x displacement.
    float x, 

    /// The y displacement.
    float y, 

    /// The z displacement.
    float z
);

/// Expresses a scaling in the x, y, and z directions.
///
void ses_matrix_scale(
    /// The transform matrix to modify.
    sesMatrix_t * m,  

    /// The x scale modifier.
    float x, 

    /// The y scale modifier.
    float y, 

    /// The z scale modifier.
    float z
);

    


#endif


