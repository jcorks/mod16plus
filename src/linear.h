#ifndef H_MOD16__LINEAR_ALGEBRA__INCLUDED
#define H_MOD16__LINEAR_ALGEBRA__INCLUDED





typedef struct {
    float x;
    float y;
    float z;
} mod16Vector_t;


/// A generic, row-major matrix 
///
typedef struct mod16Matrix_t mod16Matrix_t;
struct mod16Matrix_t {
    /// Data for the matrix, elements are laid out 
    /// linearly in memory, starting with the topleft,
    /// then laying out rows.
    float data[16];
};

/// Sets the matrix to the identity matrix, removing all transformations.
///
void mod16_matrix_set_identity(
    /// Matrix to modify.
    mod16Matrix_t * m
);

/// Convenience function that transforms the 
/// given point and returns its result.
///
mod16Vector_t mod16_matrix_transform(
    /// The matrix to use as source.
    const mod16Matrix_t * m, 

    /// The point to transform.
    const mod16Vector_t * point
);




/// Transpomod16 the matrix.
///
void mod16_matrix_transpose(
    /// The matrix to modify.
    mod16Matrix_t * m
);

/// Inverts the matrix.
///
void mod16_matrix_invert(
    /// The matrix to modify.
    mod16Matrix_t * m
);

/// Reverse the majority of the matrix.
///
void mod16_matrix_reverse_majority(
    /// The matrix to modify.
    mod16Matrix_t * m
);

/// Returns the internal representation of the TransformMatrix.
///
#define mod16_matrix_ptr(__M__) ((float*)__M__.data)


/// Multiplies 2 matrices.
///
mod16Matrix_t mod16_matrix_multiply(
    /// The first operand of the multiplication.
    const mod16Matrix_t * a, 

    /// The second operand of the multiplication.
    const mod16Matrix_t * b
);


/// Rotates the matrix about the Euler angles psi, theta, and phi.
///
void mod16_matrix_rotate_by_angles(
    /// The transform matrix to modify.
    mod16Matrix_t * m,  

    /// The psi (x) rotation angle.
    float x, 

    /// The theta (y) rotation angle.
    float y, 

    /// The phi (z) rotation angle.
    float z
);

/// Expresmod16 a translation by x, y, and z
///
void mod16_matrix_translate(
    /// The transform matrix to modify.
    mod16Matrix_t * m,  

    /// The x displacement.
    float x, 

    /// The y displacement.
    float y, 

    /// The z displacement.
    float z
);

/// Expresmod16 a scaling in the x, y, and z directions.
///
void mod16_matrix_scale(
    /// The transform matrix to modify.
    mod16Matrix_t * m,  

    /// The x scale modifier.
    float x, 

    /// The y scale modifier.
    float y, 

    /// The z scale modifier.
    float z
);

    


#endif


