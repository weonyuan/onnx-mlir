# SPDX-License-Identifier: Apache-2.0

add_onnx_mlir_interface(ShapeInferenceOpInterface)
add_onnx_mlir_interface(ShapeHelperOpInterface)
add_onnx_mlir_interface(ResultTypeInferenceOpInterface)
add_onnx_mlir_interface(HasOnnxSubgraphOpInterface)
add_onnx_mlir_interface(SpecializedKernelOpInterface)

add_onnx_mlir_library(OMShapeInferenceOpInterface
  ShapeInferenceOpInterface.cpp

  DEPENDS
  MLIRIR
  OMShapeInferenceOpInterfaceIncGen

  LINK_LIBS PUBLIC
  LLVMSupport
  )

add_onnx_mlir_library(OMShapeHelperOpInterface
  ShapeHelperOpInterface.cpp

  DEPENDS
  MLIRIR
  OMMlirDialects
  OMShapeHelperOpInterfaceIncGen

  LINK_LIBS PUBLIC
  MLIRSupport
  )

add_onnx_mlir_library(OMResultTypeInferenceOpInterface
  ResultTypeInferenceOpInterface.cpp

  DEPENDS
  MLIRIR
  OMResultTypeInferenceOpInterfaceIncGen

  LINK_LIBS PUBLIC
  LLVMSupport
  )

add_onnx_mlir_library(OMHasOnnxSubgraphOpInterface
  HasOnnxSubgraphOpInterface.cpp

  DEPENDS
  MLIRIR
  OMHasOnnxSubgraphOpInterfaceIncGen

  LINK_LIBS PUBLIC
  LLVMSupport
  )

add_onnx_mlir_library(OMSpecializedKernelOpInterface
  SpecializedKernelOpInterface.cpp

  DEPENDS
  MLIRIR
  OMSpecializedKernelOpInterfaceIncGen

  LINK_LIBS PUBLIC
  LLVMSupport
  )
