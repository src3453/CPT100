/* CPT200 custom blocks */

Blockly.Blocks['cpt_program_root'] = {
  init: function() {
    this.appendDummyInput().appendField('Program');
    this.appendStatementInput('BOOT').setCheck(null).appendField('BOOT');
    this.appendStatementInput('LOOP').setCheck(null).appendField('LOOP');
    this.setColour('#5b80a5');
    this.setTooltip('BOOTとLOOP関数の中身を組み立てます');
    this.setHelpUrl('');
  }
};

Blockly.Blocks['cpt_print'] = {
  init: function() {
    this.appendValueInput('TEXT').setCheck(['String', 'Number']).appendField('print text');
    this.appendValueInput('X').setCheck('Number').appendField('x');
    this.appendValueInput('Y').setCheck('Number').appendField('y');
    this.appendValueInput('COLOR').setCheck('Number').appendField('color');
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour('#a5a55b');
  }
};

Blockly.Blocks['cpt_pix'] = {
  init: function() {
    this.appendValueInput('X').setCheck('Number').appendField('pix x');
    this.appendValueInput('Y').setCheck('Number').appendField('y');
    this.appendValueInput('COLOR').setCheck('Number').appendField('color');
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour('#5ba58c');
  }
};

Blockly.Blocks['cpt_rect'] = {
  init: function() {
    this.appendValueInput('X').setCheck('Number').appendField('rect x');
    this.appendValueInput('Y').setCheck('Number').appendField('y');
    this.appendValueInput('W').setCheck('Number').appendField('w');
    this.appendValueInput('H').setCheck('Number').appendField('h');
    this.appendValueInput('COLOR').setCheck('Number').appendField('color');
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour('#5ba58c');
  }
};

Blockly.Blocks['cpt_line'] = {
  init: function() {
    this.appendValueInput('XS').setCheck('Number').appendField('line xs');
    this.appendValueInput('YS').setCheck('Number').appendField('ys');
    this.appendValueInput('XE').setCheck('Number').appendField('xe');
    this.appendValueInput('YE').setCheck('Number').appendField('ye');
    this.appendValueInput('COLOR').setCheck('Number').appendField('color');
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour('#5ba58c');
  }
};

Blockly.Blocks['cpt_rgb'] = {
  init: function() {
    this.appendValueInput('R').setCheck('Number').appendField('rgb r');
    this.appendValueInput('G').setCheck('Number').appendField('g');
    this.appendValueInput('B').setCheck('Number').appendField('b');
    this.setOutput(true, 'Number');
    this.setColour('#5ba58c');
  }
};

Blockly.Blocks['cpt_rgbd'] = {
  init: function() {
    this.appendValueInput('X').setCheck('Number').appendField('rgbd x');
    this.appendValueInput('Y').setCheck('Number').appendField('y');
    this.appendValueInput('R').setCheck('Number').appendField('r');
    this.appendValueInput('G').setCheck('Number').appendField('g');
    this.appendValueInput('B').setCheck('Number').appendField('b');
    this.setOutput(true, 'Number');
    this.setColour('#5ba58c');
  }
};

Blockly.Blocks['cpt_cls'] = {
  init: function() {
    this.appendValueInput('COLOR').setCheck('Number').appendField('cls color');
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour('#a55b5b');
  }
};

Blockly.Blocks['cpt_screen'] = {
  init: function() {
    this.appendValueInput('MODE').setCheck('Number').appendField('screen mode');
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour('#a55b80');
  }
};

Blockly.Blocks['cpt_spr'] = {
  init: function() {
    this.appendValueInput('INDEX').setCheck('Number').appendField('spr index');
    this.appendDummyInput().appendField('enabled').appendField(new Blockly.FieldCheckbox('TRUE'), 'ENABLED');
    this.appendValueInput('X').setCheck('Number').appendField('x');
    this.appendValueInput('Y').setCheck('Number').appendField('y');
    this.appendValueInput('ROT').setCheck('Number').appendField('rotation');
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour('#8c5ba5');
  }
};

Blockly.Blocks['cpt_time'] = {
  init: function() {
    this.appendDummyInput().appendField('time()');
    this.setOutput(true, 'Number');
    this.setColour('#a55b5b');
  }
};

Blockly.Blocks['cpt_key'] = {
  init: function() {
    this.appendValueInput('KEYCODE').setCheck('Number').appendField('key keycode');
    this.setOutput(true, 'Number');
    this.setColour('#5b8ca5');
  }
};

Blockly.Blocks['cpt_mouse_assign'] = {
  init: function() {
    this.appendDummyInput().appendField('mouse() → ').appendField(new Blockly.FieldTextInput('mx'), 'VX')
      .appendField(',').appendField(new Blockly.FieldTextInput('my'), 'VY')
      .appendField(',').appendField(new Blockly.FieldTextInput('mb'), 'VB');
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour('#5b8ca5');
    this.setTooltip('mouse()の3値をローカル変数へ代入');
  }
};
