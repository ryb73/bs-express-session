open Express;

let resultToOpt = (r) =>
    switch r {
        | Belt.Result.Ok(v) => Some(v)
        | Error(_) => None
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
        -> Js.Dict.get("session");

    let _getSessionDict = (req) => Belt.Option.(
        _getSession(req)
        -> flatMap(Js.Json.decodeObject)
    );

    let _getSessionObj = (req) : _session =>
        _getSession(req)
        |> Obj.magic;

    let set = (req, value) => Belt.Option.(
        _getSessionDict(req)
        -> map((session) => Js.Dict.set(session, C.key, C.t_encode(value)))
        != None
    );

    let get = (req) => Belt.Option.(
        _getSessionDict(req)
        -> flatMap(dict => Js.Dict.get(dict, C.key))
        -> flatMap((json) => resultToOpt(C.t_decode(json)))
    );

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
