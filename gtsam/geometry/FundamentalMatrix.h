/*
 * @file FundamentalMatrix.h
 * @brief FundamentalMatrix classes
 * @author Frank Dellaert
 * @date Oct 23, 2024
 */

#pragma once

#include <gtsam/geometry/EssentialMatrix.h>
#include <gtsam/geometry/Rot3.h>
#include <gtsam/geometry/Unit3.h>

namespace gtsam {

class FundamentalMatrix {
 private:
  Rot3 U_;    ///< Left rotation
  double s_;  ///< Scalar parameter for S
  Rot3 V_;    ///< Right rotation

 public:
  /// Default constructor
  FundamentalMatrix() : U_(Rot3()), s_(1.0), V_(Rot3()) {}

  /// Construct from U, V, and scalar s
  FundamentalMatrix(const Rot3& U, double s, const Rot3& V)
      : U_(U), s_(s), V_(V) {}

  /// Return the fundamental matrix representation
  Matrix3 matrix() const {
    return U_.matrix() * Vector3(1, s_, 1).asDiagonal() *
           V_.transpose().matrix();
  }

  /// @name Testable
  /// @{

  /// Print the FundamentalMatrix
  void print(const std::string& s = "") const {
    std::cout << s << "U:\n"
              << U_.matrix() << "\ns: " << s_ << "\nV:\n"
              << V_.matrix() << std::endl;
  }

  /// Check if the FundamentalMatrix is equal to another within a tolerance
  bool equals(const FundamentalMatrix& other, double tol = 1e-9) const {
    return U_.equals(other.U_, tol) && std::abs(s_ - other.s_) < tol &&
           V_.equals(other.V_, tol);
  }

  /// @}

  /// @name Manifold
  /// @{
  enum { dimension = 7 };  // 3 for U, 1 for s, 3 for V
  inline static size_t Dim() { return dimension; }
  inline size_t dim() const { return dimension; }

  /// Return local coordinates with respect to another FundamentalMatrix
  Vector localCoordinates(const FundamentalMatrix& F) const {
    Vector result(7);
    result.head<3>() = U_.localCoordinates(F.U_);
    result(3) = F.s_ - s_;  // Difference in scalar
    result.tail<3>() = V_.localCoordinates(F.V_);
    return result;
  }

  /// Retract the given vector to get a new FundamentalMatrix
  FundamentalMatrix retract(const Vector& delta) const {
    Rot3 newU = U_.retract(delta.head<3>());
    double newS = s_ + delta(3);  // Update scalar
    Rot3 newV = V_.retract(delta.tail<3>());
    return FundamentalMatrix(newU, newS, newV);
  }

  /// @}
};

/**
 * @class SimpleFundamentalMatrix
 * @brief Class for representing a simple fundamental matrix.
 *
 * This class represents a simple fundamental matrix, which is a
 * parameterization of the essential matrix and focal lengths for left and right
 * cameras. Principal points are not part of the manifold but a convenience.
 */
class SimpleFundamentalMatrix {
 private:
  EssentialMatrix E_;  ///< Essential matrix
  double fa_;          ///< Focal length for left camera
  double fb_;          ///< Focal length for right camera
  Point2 ca_;          ///< Principal point for left camera
  Point2 cb_;          ///< Principal point for right camera

 public:
  /// Default constructor
  SimpleFundamentalMatrix()
      : E_(), fa_(1.0), fb_(1.0), ca_(0.0, 0.0), cb_(0.0, 0.0) {}

  /// Construct from essential matrix and focal lengths
  SimpleFundamentalMatrix(const EssentialMatrix& E,  //
                          double fa, double fb,
                          const Point2& ca = Point2(0.0, 0.0),
                          const Point2& cb = Point2(0.0, 0.0))
      : E_(E), fa_(fa), fb_(fb), ca_(ca), cb_(cb) {}

  /// Return the fundamental matrix representation
  Matrix3 matrix() const {
    Matrix3 Ka, Kb;
    Ka << fa_, 0, ca_.x(), 0, fa_, ca_.y(), 0, 0, 1;  // Left calibration
    Kb << fb_, 0, cb_.x(), 0, fb_, cb_.y(), 0, 0, 1;  // Right calibration
    return Ka * E_.matrix() * Kb.inverse();
  }

  /// @name Testable
  /// @{

  /// Print the SimpleFundamentalMatrix
  void print(const std::string& s = "") const {
    std::cout << s << "E:\n"
              << E_.matrix() << "\nfa: " << fa_ << "\nfb: " << fb_
              << "\nca: " << ca_ << "\ncb: " << cb_ << std::endl;
  }

  /// Check equality within a tolerance
  bool equals(const SimpleFundamentalMatrix& other, double tol = 1e-9) const {
    return E_.equals(other.E_, tol) && std::abs(fa_ - other.fa_) < tol &&
           std::abs(fb_ - other.fb_) < tol && (ca_ - other.ca_).norm() < tol &&
           (cb_ - other.cb_).norm() < tol;
  }
  /// @}

  /// @name Manifold
  /// @{
  enum { dimension = 7 };  // 5 for E, 1 for fa, 1 for fb
  inline static size_t Dim() { return dimension; }
  inline size_t dim() const { return dimension; }

  /// Return local coordinates with respect to another
  /// SimpleFundamentalMatrix
  Vector localCoordinates(const SimpleFundamentalMatrix& F) const {
    Vector result(7);
    result.head<5>() = E_.localCoordinates(F.E_);
    result(5) = F.fa_ - fa_;  // Difference in fa
    result(6) = F.fb_ - fb_;  // Difference in fb
    return result;
  }

  /// Retract the given vector to get a new SimpleFundamentalMatrix
  SimpleFundamentalMatrix retract(const Vector& delta) const {
    EssentialMatrix newE = E_.retract(delta.head<5>());
    double newFa = fa_ + delta(5);  // Update fa
    double newFb = fb_ + delta(6);  // Update fb
    return SimpleFundamentalMatrix(newE, newFa, newFb, ca_, cb_);
  }
  /// @}
};

template <>
struct traits<FundamentalMatrix>
    : public internal::Manifold<FundamentalMatrix> {};

template <>
struct traits<SimpleFundamentalMatrix>
    : public internal::Manifold<SimpleFundamentalMatrix> {};

}  // namespace gtsam
