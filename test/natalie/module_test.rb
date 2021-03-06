require_relative '../spec_helper'

module M1
end

module M2
  include M1
end

class C1
  include M1
end

describe 'Module' do
  describe '.include?' do
    it 'returns true if the module includes the given module' do
      M2.include?(M2).should == false
      M2.include?(M1).should == true
      M1.include?(M2).should == false
      C1.include?(M1).should == true
      C1.include?(M2).should == false
      -> { C1.include?(nil) }.should raise_error(TypeError)
    end
  end
end
