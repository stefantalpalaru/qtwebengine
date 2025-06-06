// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_REGEXP_ARM_REGEXP_MACRO_ASSEMBLER_ARM_H_
#define V8_REGEXP_ARM_REGEXP_MACRO_ASSEMBLER_ARM_H_

#include "src/codegen/arm/assembler-arm.h"
#include "src/codegen/macro-assembler.h"
#include "src/regexp/regexp-macro-assembler.h"

namespace v8 {
namespace internal {

class V8_EXPORT_PRIVATE RegExpMacroAssemblerARM
    : public NativeRegExpMacroAssembler {
 public:
  RegExpMacroAssemblerARM(Isolate* isolate, Zone* zone, Mode mode,
                          int registers_to_save);
  virtual ~RegExpMacroAssemblerARM();
  virtual int stack_limit_slack();
  virtual void AdvanceCurrentPosition(int by);
  virtual void AdvanceRegister(int reg, int by);
  virtual void Backtrack();
  virtual void Bind(Label* label);
  virtual void CheckAtStart(int cp_offset, Label* on_at_start);
  virtual void CheckCharacter(unsigned c, Label* on_equal);
  virtual void CheckCharacterAfterAnd(unsigned c,
                                      unsigned mask,
                                      Label* on_equal);
  virtual void CheckCharacterGT(uc16 limit, Label* on_greater);
  virtual void CheckCharacterLT(uc16 limit, Label* on_less);
  // A "greedy loop" is a loop that is both greedy and with a simple
  // body. It has a particularly simple implementation.
  virtual void CheckGreedyLoop(Label* on_tos_equals_current_position);
  virtual void CheckNotAtStart(int cp_offset, Label* on_not_at_start);
  virtual void CheckNotBackReference(int start_reg, bool read_backward,
                                     Label* on_no_match);
  virtual void CheckNotBackReferenceIgnoreCase(int start_reg,
                                               bool read_backward, bool unicode,
                                               Label* on_no_match);
  virtual void CheckNotCharacter(unsigned c, Label* on_not_equal);
  virtual void CheckNotCharacterAfterAnd(unsigned c,
                                         unsigned mask,
                                         Label* on_not_equal);
  virtual void CheckNotCharacterAfterMinusAnd(uc16 c,
                                              uc16 minus,
                                              uc16 mask,
                                              Label* on_not_equal);
  virtual void CheckCharacterInRange(uc16 from,
                                     uc16 to,
                                     Label* on_in_range);
  virtual void CheckCharacterNotInRange(uc16 from,
                                        uc16 to,
                                        Label* on_not_in_range);
  virtual void CheckBitInTable(Handle<ByteArray> table, Label* on_bit_set);

  // Checks whether the given offset from the current position is before
  // the end of the string.
  virtual void CheckPosition(int cp_offset, Label* on_outside_input);
  virtual bool CheckSpecialCharacterClass(uc16 type,
                                          Label* on_no_match);
  virtual void Fail();
  virtual Handle<HeapObject> GetCode(Handle<String> source);
  virtual void GoTo(Label* label);
  virtual void IfRegisterGE(int reg, int comparand, Label* if_ge);
  virtual void IfRegisterLT(int reg, int comparand, Label* if_lt);
  virtual void IfRegisterEqPos(int reg, Label* if_eq);
  virtual IrregexpImplementation Implementation();
  virtual void LoadCurrentCharacterUnchecked(int cp_offset,
                                             int character_count);
  virtual void PopCurrentPosition();
  virtual void PopRegister(int register_index);
  virtual void PushBacktrack(Label* label);
  virtual void PushCurrentPosition();
  virtual void PushRegister(int register_index,
                            StackCheckFlag check_stack_limit);
  virtual void ReadCurrentPositionFromRegister(int reg);
  virtual void ReadStackPointerFromRegister(int reg);
  virtual void SetCurrentPositionFromEnd(int by);
  virtual void SetRegister(int register_index, int to);
  virtual bool Succeed();
  virtual void WriteCurrentPositionToRegister(int reg, int cp_offset);
  virtual void ClearRegisters(int reg_from, int reg_to);
  virtual void WriteStackPointerToRegister(int reg);

  // Called from RegExp if the stack-guard is triggered.
  // If the code object is relocated, the return address is fixed before
  // returning.
  // {raw_code} is an Address because this is called via ExternalReference.
  static int CheckStackGuardState(Address* return_address, Address raw_code,
                                  Address re_frame, uintptr_t extra_space);

 private:
  // Offsets from frame_pointer() of function parameters and stored registers.
  static const int kFramePointer = 0;

  // Above the frame pointer - Stored registers and stack passed parameters.
  // Register 4..11.
  static const int kStoredRegisters = kFramePointer;
  // Return address (stored from link register, read into pc on return).
  static const int kReturnAddress = kStoredRegisters + 8 * kPointerSize;
  // Stack parameters placed by caller.
  static const int kRegisterOutput = kReturnAddress + kPointerSize;
  static const int kNumOutputRegisters = kRegisterOutput + kPointerSize;
  static const int kStackHighEnd = kNumOutputRegisters + kPointerSize;
  static const int kDirectCall = kStackHighEnd + kPointerSize;
  static const int kIsolate = kDirectCall + kPointerSize;

  // Below the frame pointer.
  // Register parameters stored by setup code.
  static const int kInputEnd = kFramePointer - kPointerSize;
  static const int kInputStart = kInputEnd - kPointerSize;
  static const int kStartIndex = kInputStart - kPointerSize;
  static const int kInputString = kStartIndex - kPointerSize;
  // When adding local variables remember to push space for them in
  // the frame in GetCode.
  static const int kSuccessfulCaptures = kInputString - kPointerSize;
  static const int kStringStartMinusOne = kSuccessfulCaptures - kPointerSize;
  static const int kBacktrackCount = kStringStartMinusOne - kSystemPointerSize;
  // First register address. Following registers are below it on the stack.
  static const int kRegisterZero = kBacktrackCount - kSystemPointerSize;

  // Initial size of code buffer.
  static const int kRegExpCodeSize = 1024;

  static const int kBacktrackConstantPoolSize = 4;

  // Check whether preemption has been requested.
  void CheckPreemption();

  // Check whether we are exceeding the stack limit on the backtrack stack.
  void CheckStackLimit();


  // Generate a call to CheckStackGuardState.
  void CallCheckStackGuardState(
      Operand extra_space_for_variables = Operand::Zero());

  // The ebp-relative location of a regexp register.
  MemOperand register_location(int register_index);

  // Register holding the current input position as negative offset from
  // the end of the string.
  inline Register current_input_offset() { return r6; }

  // The register containing the current character after LoadCurrentCharacter.
  inline Register current_character() { return r7; }

  // Register holding address of the end of the input string.
  inline Register end_of_input_address() { return r10; }

  // Register holding the frame address. Local variables, parameters and
  // regexp registers are addressed relative to this.
  inline Register frame_pointer() { return fp; }

  // The register containing the backtrack stack top. Provides a meaningful
  // name to the register.
  inline Register backtrack_stackpointer() { return r8; }

  // Register holding pointer to the current code object.
  inline Register code_pointer() { return r5; }

  // Byte size of chars in the string to match (decided by the Mode argument)
  inline int char_size() { return static_cast<int>(mode_); }

  // Equivalent to a conditional branch to the label, unless the label
  // is nullptr, in which case it is a conditional Backtrack.
  void BranchOrBacktrack(Condition condition, Label* to);

  // Call and return internally in the generated code in a way that
  // is GC-safe (i.e., doesn't leave absolute code addresses on the stack)
  inline void SafeCall(Label* to, Condition cond = al);
  inline void SafeReturn();
  inline void SafeCallTarget(Label* name);

  // Pushes the value of a register on the backtrack stack. Decrements the
  // stack pointer by a word size and stores the register's value there.
  inline void Push(Register source);

  // Pops a value from the backtrack stack. Reads the word at the stack pointer
  // and increments it by a word size.
  inline void Pop(Register target);

  Isolate* isolate() const { return masm_->isolate(); }

  MacroAssembler* masm_;

  // Which mode to generate code for (Latin1 or UC16).
  Mode mode_;

  // One greater than maximal register index actually used.
  int num_registers_;

  // Number of registers to output at the end (the saved registers
  // are always 0..num_saved_registers_-1)
  int num_saved_registers_;

  // Labels used internally.
  Label entry_label_;
  Label start_label_;
  Label success_label_;
  Label backtrack_label_;
  Label exit_label_;
  Label check_preempt_label_;
  Label stack_overflow_label_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_REGEXP_ARM_REGEXP_MACRO_ASSEMBLER_ARM_H_
