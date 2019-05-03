open BsAbstract.Interface;

module OfEq = (E: EQ) => {
  include Relude_Array_Types.FoldableOfEq(E);
  let distinct = Relude_Array_Base.distinctBy(E.eq);
  let removeFirst = Relude_Array_Base.removeFirstBy(E.eq);
  let removeEach = Relude_Array_Base.removeEachBy(E.eq);
  let eq = Relude_Array_Types.eqBy(E.eq);
};

module OfOrd = (O: ORD) => {
  include OfEq(O);
  include Relude_Array_Types.FoldableOfOrd(O);
  let sort = Relude_Array_Base.sortBy(O.compare);
};

module OfMonoid = (M: MONOID) => {
  include Relude_Array_Types.FoldableOfMonoid(M);
};

module String = {
  include OfOrd(Relude_String.Ord);
  include OfMonoid(Relude_String.Monoid);
  let join = fold;
  let joinWith = intercalate;

  /**
   * Specialized `distinct` function that removes duplicate strings in O(n)
   * time by using `Js.Dict`.
   */
  let distinct = xs =>
    Relude_Array_Types.foldLeft(
      (acc, curr) => {
        Js.Dict.set(acc, curr, 0);
        acc;
      },
      Js.Dict.empty(),
      xs,
    )
    |> Js.Dict.keys;
};

module Int = {
  include OfOrd(Relude_Int.Ord);
  let sum = Relude_Array_Types.fold((module Relude_Int.Additive.Monoid));
  let product =
    Relude_Array_Types.fold((module Relude_Int.Multiplicative.Monoid));
};

module Float = {
  include OfOrd(Relude_Float.Ord);
  let sum = Relude_Array_Types.fold((module Relude_Float.Additive.Monoid));
  let product =
    Relude_Array_Types.fold((module Relude_Float.Multiplicative.Monoid));
};

module Option = {
  include Relude_Array_Types.Traversable(Relude_Option.Applicative);
};

module Result = {
  let traverse = (type e, f, xs) => {
    module ResultFixedError =
      Relude_Result.Applicative({
        type t = e;
      });
    module TraverseResult = Relude_Array_Types.Traversable(ResultFixedError);
    TraverseResult.traverse(f, xs);
  };

  let sequence = (type e, xs) => {
    module ResultFixedError =
      Relude_Result.Applicative({
        type t = e;
      });
    module TraverseResult = Relude_Array_Types.Traversable(ResultFixedError);
    TraverseResult.sequence(xs);
  };
};

module Validation = {
  module Traversable =
         (
           Errors: BsAbstract.Interface.SEMIGROUP_ANY,
           Error: BsAbstract.Interface.TYPE,
         ) =>
    BsAbstract.Array.Traversable(
      (Relude_Validation.Applicative(Errors, Error)),
    );

  module TraversableWithErrorsAsArray = (Error: BsAbstract.Interface.TYPE) =>
    Traversable(Relude_Array_Types.SemigroupAny, Error);

  module TraversableWithErrorsAsArrayOfStrings =
    TraversableWithErrorsAsArray({
      type t = string;
    });

  module TraversableWithErrorsAsNonEmptyArray =
         (Error: BsAbstract.Interface.TYPE) =>
    Traversable(Relude_NonEmpty.Array.SemigroupAny, Error);

  /*
   This is a streamlined definition of traverse which allows you to return a Belt.Result.t for each item
   in the array, and all errors are collected in a NonEmpty.Array of your error type, using applicative semantics
   for Validation.
   */
  let traverse =
      (
        type a,
        type b,
        type e,
        f: a => Belt.Result.t(b, e), /* Each a produces a Result with a success value or a single error value */
        array: array(a),
      )
      : Relude_Validation.t(array(b), Relude_NonEmpty.Array.t(e)) => {
    module Error = {
      type t = e;
    };
    module Traversable =
      Traversable(Relude_NonEmpty.Array.SemigroupAny, Error);
    /* Map the reuslts to Validation.t(a, NonEmpty.Array.t(e)), so we can accumulate the errors */
    Traversable.traverse(a => f(a)->Relude_Result.toValidationNea, array);
  };
};