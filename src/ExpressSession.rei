type cookieOpts;
let cookieOpts:
    (~domain: string=?, ~httpOnly: bool=?, ~maxAge: int=?, ~path: string=?,
    ~secure: bool=?, unit) => cookieOpts;
type opts;
let opts:
    (~secret: string, ~cookie: cookieOpts=?,
    ~genid: Express.Request.t => string=?, ~name: string=?, ~proxy: bool=?,
    ~resave: bool=?, ~rolling: bool=?, ~saveUninitialized: bool=?, unit) =>
    opts;
let make: opts => Express.Middleware.t;
module type Config = {
    type t;
    let t_encode: t => Js.Json.t;
    let t_decode: Js.Json.t => Belt.Result.t(t, Decco.decodeError);
    let key: string;
};
module Make: (C : Config) => {
    let set: (Express.Request.t, C.t) => bool;
    let get: Express.Request.t => option(C.t);
    let destroy: Express.Request.t => Js.Promise.t(unit);
};
