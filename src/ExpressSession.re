open Express;
open Option;
open Belt.Result;

let flip = BatPervasives.flip;

let _resultToOpt = (r) =>
    switch r {
        | Ok(v) => Some(v)
        | _ => None
    };

type cookieOpts;

[@bs.obj]
external cookieOpts : (
    ~domain: string=?, ~httpOnly: bool=?, ~maxAge: int=?,
    ~path: string=?, ~secure: bool=?, unit
) => cookieOpts = "";

type opts;

[@bs.obj]
external opts : (
    ~secret: string,
    ~cookie: cookieOpts=?,
    ~genid: Request.t => string=?,
    ~name: string=?,
    ~proxy: bool=?,
    ~resave: bool=?,
    ~rolling: bool=?,
    ~saveUninitialized: bool=?,
    unit
) => opts = "";

[@bs.module] external make : opts => Middleware.t = "express-session";

module type Config = {
    [@decco] type t;
    let key: string;
};

module Make = (C: Config) => {
    type _session;
    let _getSession = (req) =>
        Request.asJsonObject(req)
            |> flip(Js.Dict.get, "session");

    let _getSessionDict = (req) =>
        _getSession(req)
            |> flip(bind, Js.Json.decodeObject);

    let _getSessionObj = (req) : _session =>
        _getSession(req)
            |> Obj.magic;

    let set = (req, value) =>
        _getSessionDict(req)
            |> Option.map((session) => Js.Dict.set(session, C.key, C.t_encode(value)))
            != None;

    let get = (req) =>
        _getSessionDict(req)
            |> flip(bind, flip(Js.Dict.get, C.key))
            |> flip(bind, (json) => _resultToOpt(C.t_decode(json)));

    [@bs.send.pipe: _session] external _destroy : ((Js.nullable(exn)) => unit) => unit = "destroy";
    let destroy = (req) =>
        Js.Promise.make((~resolve, ~reject) => {
            _getSessionObj(req)
                |> _destroy((exn) => {
                    switch (Js.Nullable.toOption(exn)) {
                        | Some(exn) => [@bs] reject(exn)
                        | _ => let u = (); [@bs] resolve(u)
                    };
                });
        });
};
