diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var a: i32 = 0;
    var b: i32 = 0;
    var c: i32 = 0;
    var d: i32 = 0;
    a = 1;
    b = 2;
    c = 5;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((a == 1) && (b == 2)) && (c == 5)) && (d == 0)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
