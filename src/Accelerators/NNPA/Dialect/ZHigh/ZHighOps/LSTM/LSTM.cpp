/*
 * SPDX-License-Identifier: Apache-2.0
 */

//===------------------ LSTM.cpp - ZHigh Operations -----------------------===//
//
// Copyright 2019-2022 The IBM Research Authors.
//
// =============================================================================
//
//
//===----------------------------------------------------------------------===//

#include "src/Accelerators/NNPA/Dialect/ZHigh/ZHighOps/ShapeHelper.hpp"

using namespace mlir;
using namespace onnx_mlir;

namespace onnx_mlir {
namespace zhigh {

//===----------------------------------------------------------------------===//
// ShapeHelper
//===----------------------------------------------------------------------===//

LogicalResult ZHighLSTMOpShapeHelper::computeShape() {
  ZHighLSTMOp lstmOp = llvm::dyn_cast<ZHighLSTMOp>(op);
  ZHighLSTMOp::Adaptor operandAdaptor(operands);
  // Get operands.
  // X: [S, B, I]
  Value X = operandAdaptor.input();
  // R: [D, H, H]
  Value R = operandAdaptor.hidden_weights();

  // Return all timesteps or only the final step;
  bool isAllTimesteps = (lstmOp.return_all_steps() == -1) ? true : false;

  // Get bounds
  SmallVector<IndexExpr, 4> XDims, RDims;
  createIE->getShapeAsDims(X, XDims);
  createIE->getShapeAsDims(R, RDims);
  IndexExpr S = XDims[0];
  IndexExpr B = XDims[1];
  IndexExpr I = XDims[2];
  IndexExpr D = RDims[0];
  IndexExpr H = RDims[1];

  // Shape for hn_ouput : [S, D, B, H] if return all  timesteps. [1, D, B, H] if
  // return the final step only.
  DimsExpr hnOutputDims;
  if (isAllTimesteps)
    hnOutputDims.emplace_back(S);
  else
    hnOutputDims.emplace_back(LiteralIndexExpr(1));
  hnOutputDims.emplace_back(D);
  hnOutputDims.emplace_back(B);
  hnOutputDims.emplace_back(H);

  // Shape for cf_ouput : [1, D, B, H]
  DimsExpr cfOutputDims;
  cfOutputDims.emplace_back(LiteralIndexExpr(1));
  cfOutputDims.emplace_back(D);
  cfOutputDims.emplace_back(B);
  cfOutputDims.emplace_back(H);

  // Shape for optional values.
  // Initialized value: [D, B, H]
  hc0Shape.emplace_back(D);
  hc0Shape.emplace_back(B);
  hc0Shape.emplace_back(H);
  // Bias value: [D, 4*H]
  biasShape.emplace_back(D);
  biasShape.emplace_back(H * 4);

  // Keep all original dimensions.
  allOriginalDims.emplace_back(D);
  allOriginalDims.emplace_back(S);
  allOriginalDims.emplace_back(B);
  allOriginalDims.emplace_back(I);
  allOriginalDims.emplace_back(H);

  // Save the final results.
  setOutputDims(hnOutputDims, 0);
  setOutputDims(cfOutputDims, 1);
  return success();
}

//===----------------------------------------------------------------------===//
// Verifier
//===----------------------------------------------------------------------===//

LogicalResult ZHighLSTMOp::verify() {
  ZHighLSTMOpAdaptor operandAdaptor(*this);
  // Get operands.
  Value W = operandAdaptor.input_weights();
  Value R = operandAdaptor.hidden_weights();
  Value WB = operandAdaptor.input_bias();
  Value RB = operandAdaptor.hidden_bias();

  // Hidden size attribute.
  int64_t hiddenSize = hidden_size();

  // Verify hidden size in W.
  if (hasRankedType(W)) {
    int64_t dim2 = W.getType().cast<RankedTensorType>().getShape()[2];
    if (!ShapedType::isDynamic(dim2) && (dim2 != hiddenSize * 4))
      return failure();
  }

  // Verify hidden size in R.
  if (hasRankedType(R)) {
    int64_t dim1 = R.getType().cast<RankedTensorType>().getShape()[1];
    int64_t dim2 = R.getType().cast<RankedTensorType>().getShape()[2];
    if (!ShapedType::isDynamic(dim1) && (dim1 != hiddenSize))
      return failure();
    if (!ShapedType::isDynamic(dim2) && (dim2 != hiddenSize * 4))
      return failure();
  }

  // Verify hidden size in WB.
  if (!WB.getType().isa<NoneType>() && hasRankedType(WB)) {
    int64_t dim1 = WB.getType().cast<RankedTensorType>().getShape()[1];
    if (!ShapedType::isDynamic(dim1) && (dim1 != hiddenSize * 4))
      return failure();
  }

  // Verify hidden size in RB.
  if (!RB.getType().isa<NoneType>() && hasRankedType(RB)) {
    int64_t dim1 = RB.getType().cast<RankedTensorType>().getShape()[1];
    if (!ShapedType::isDynamic(dim1) && (dim1 != hiddenSize * 4))
      return failure();
  }

  return success();
}

//===----------------------------------------------------------------------===//
// Shape inference
//===----------------------------------------------------------------------===//

LogicalResult ZHighLSTMOp::inferShapes(
    std::function<void(mlir::Region &)> doShapeInference) {
  if (!hasRankedType(input()) || !hasRankedType(hidden_weights()))
    return success();

  ZHighLSTMOpShapeHelper shapeHelper(getOperation());
  shapeHelper.computeShapeAndAssertOnFailure();

  // Output type is 4DS.
  SmallVector<int64_t, 4> hnOutputDims, cfOutputDims;
  IndexExpr::getShape(shapeHelper.getOutputDims(0), hnOutputDims);
  IndexExpr::getShape(shapeHelper.getOutputDims(1), cfOutputDims);
  Type elementType = input().getType().cast<ShapedType>().getElementType();
  ZTensorEncodingAttr encoding = ZTensorEncodingAttr::get(
      this->getContext(), ZTensorEncodingAttr::DataLayout::_4DS);
  updateType(getResults()[0], hnOutputDims, elementType, encoding);
  updateType(getResults()[1], cfOutputDims, elementType, encoding);
  return success();
}

} // namespace zhigh
} // namespace onnx_mlir
