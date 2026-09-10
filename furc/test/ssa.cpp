// NOLINTBEGIN(readability-identifier-naming)

#include "gtest/gtest.h"
#include <furc/middle/ir.hpp>
#include <furc/middle/ssa.hpp>
#include <unordered_set>
#include <vector>

namespace furc::test {

class SsaCfg : public testing::Test {
public:
    SsaCfg()
      : p_context(&p_function) {}
protected:
    void compute() { ssa::compute_cfg(p_function.blocks, p_cfgBlocks); }
protected:
    std::vector<ssa::cfg_block> p_cfgBlocks;

    ir_function p_function;
    ir_context  p_context;
};

TEST_F(SsaCfg, Diamond) {
    ir_operand null = { ir_operand::Integer, static_cast<std::uint64_t>(0) };

    // Entry
    p_context.terminate(null, 1, 2);
    p_context.new_next();
    // A
    p_context.terminate(3);
    p_context.new_next();
    // B
    p_context.terminate(3);
    p_context.new_next();
    // C
    p_context.terminate(4);
    p_context.new_next();
    // Exit
    p_context.terminate();

    compute();

    EXPECT_EQ(p_cfgBlocks[0].sucs.size(), 2);
    EXPECT_NE(p_cfgBlocks[0].sucs.find(1), p_cfgBlocks[0].sucs.end());
    EXPECT_NE(p_cfgBlocks[0].sucs.find(2), p_cfgBlocks[0].sucs.end());
    EXPECT_EQ(p_cfgBlocks[3].preds, p_cfgBlocks[0].sucs);
    EXPECT_EQ(p_cfgBlocks[3].sucs.size(), 1);
    EXPECT_NE(p_cfgBlocks[3].sucs.find(4), p_cfgBlocks[3].sucs.end());
}

class SsaDom : public testing::Test {
public:
    SsaDom()
      : p_context(&p_function) {}
protected:
    void compute() {
        ssa::compute_cfg(p_function.blocks, p_cfgBlocks);
        p_ssaRegisters.resize(p_function.regCount);
        ssa::collect_registers(p_function.blocks, p_ssaRegisters, p_ssaGlobals);

        std::vector<std::uint64_t> order;
        ssa::compute_rpo(p_cfgBlocks, p_ssaBlocks, order);

        ssa::build_dtree(p_cfgBlocks, p_ssaBlocks, order);
        ssa::compute_dfrontiers(p_cfgBlocks, p_ssaBlocks);

        ssa::ssaification(p_function.blocks, p_cfgBlocks, p_ssaBlocks, p_ssaRegisters, p_ssaGlobals);
        ssa::rename(p_function.blocks, p_function.regCount, p_cfgBlocks, p_ssaBlocks, order);
    }
protected:
    std::vector<ssa::cfg_block>       p_cfgBlocks;
    std::vector<ssa::ssa_block>       p_ssaBlocks;
    std::vector<ssa::register_info>   p_ssaRegisters;
    std::unordered_set<std::uint64_t> p_ssaGlobals;

    ir_function p_function;
    ir_context  p_context;
};

TEST_F(SsaDom, Diamond) {
    ir_operand null = { ir_operand::Integer, static_cast<std::uint64_t>(0) };

    // Entry
    p_context.terminate(null, 1, 2);
    p_context.new_next();
    // A
    p_context.add_instr(ir_instruction{ ir_instruction::Move,
        ir_operand{ ir_operand::Register, static_cast<std::uint64_t>(0) },
        { null } });
    p_context.terminate(3);
    p_context.new_next();
    // B
    p_context.add_instr(ir_instruction{ ir_instruction::Move,
        ir_operand{ ir_operand::Register, static_cast<std::uint64_t>(0) },
        { null } });
    p_context.terminate(3);
    p_context.new_next();
    // C
    p_context.add_instr(ir_instruction{ ir_instruction::Add,
        ir_operand{ ir_operand::Register, static_cast<std::uint64_t>(0) },
        { ir_operand{ ir_operand::Register, static_cast<std::uint64_t>(0) } } });
    p_context.new_next();
    // Exit
    p_context.terminate();

    p_function.regCount = 1;

    compute();

    EXPECT_TRUE(p_ssaBlocks[0].df.empty());

    EXPECT_EQ(p_ssaBlocks[1].idom, 0);
    EXPECT_EQ(p_ssaBlocks[1].df.size(), 1);
    EXPECT_NE(p_ssaBlocks[1].df.find(3), p_ssaBlocks[1].df.end());

    EXPECT_EQ(p_ssaBlocks[2].idom, 0);
    EXPECT_EQ(p_ssaBlocks[2].df.size(), 1);
    EXPECT_NE(p_ssaBlocks[2].df.find(3), p_ssaBlocks[2].df.end());

    EXPECT_EQ(p_ssaBlocks[3].idom, 0);
    EXPECT_TRUE(p_ssaBlocks[3].df.empty());

    EXPECT_EQ(p_ssaBlocks[4].idom, 3);
    EXPECT_TRUE(p_ssaBlocks[4].df.empty());

    // Renaming
    EXPECT_EQ(p_function.blocks[1].instructions.front(),
        (ir_instruction{ ir_instruction::Move, ir_operand::reg(0, 3), { null } }));
    EXPECT_EQ(p_function.blocks[2].instructions.front(),
        (ir_instruction{ ir_instruction::Move, ir_operand::reg(0, 2), { null } }));
    EXPECT_EQ(p_function.blocks[3].instructions.front(),
        (ir_instruction{ ir_instruction::Phi,
            ir_operand::reg(0, 0),
            { ir_operand{ ir_operand::PhiPair, ir_operand::value_u::register_s{ 0, 2 }, 2 },
                ir_operand{ ir_operand::PhiPair, ir_operand::value_u::register_s{ 0, 3 }, 1 } } }));
}

} // namespace furc::test

// NOLINTEND(readability-identifier-naming)
